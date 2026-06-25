#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SHAPES 20
#define MAX_CELLS_PER_SHAPE 12
#define MAX_ORIENTS 8
#define MAX_W 55
#define MAX_H 55
#define MAX_TYPES 20

// A shape orientation: set of (dr, dc) offsets from anchor (top-left cell)
typedef struct {
    int dr[MAX_CELLS_PER_SHAPE];
    int dc[MAX_CELLS_PER_SHAPE];
    int n;         // number of cells
    int maxR;      // max row offset
    int maxC;      // max col offset
} Orient;

// All orientations for a base shape
typedef struct {
    Orient orients[MAX_ORIENTS];
    int num_orients;
    int num_cells; // cells per piece
} ShapeSet;

int num_shapes = 0;
ShapeSet shapes[MAX_SHAPES];

// For parsing raw shape cells
typedef struct {
    int r[MAX_CELLS_PER_SHAPE];
    int c[MAX_CELLS_PER_SHAPE];
    int n;
} RawShape;

static void normalize_raw(RawShape *s) {
    int minr = 999, minc = 999;
    for (int i = 0; i < s->n; i++) {
        if (s->r[i] < minr) minr = s->r[i];
        if (s->c[i] < minc) minc = s->c[i];
    }
    for (int i = 0; i < s->n; i++) {
        s->r[i] -= minr;
        s->c[i] -= minc;
    }
    // Sort for canonical form
    for (int i = 0; i < s->n - 1; i++)
        for (int j = i + 1; j < s->n; j++)
            if (s->r[j] < s->r[i] || (s->r[j] == s->r[i] && s->c[j] < s->c[i])) {
                int t = s->r[i]; s->r[i] = s->r[j]; s->r[j] = t;
                t = s->c[i]; s->c[i] = s->c[j]; s->c[j] = t;
            }
}

static int same_raw(RawShape *a, RawShape *b) {
    if (a->n != b->n) return 0;
    for (int i = 0; i < a->n; i++)
        if (a->r[i] != b->r[i] || a->c[i] != b->c[i]) return 0;
    return 1;
}

static RawShape rotate90(RawShape *s) {
    RawShape t; t.n = s->n;
    for (int i = 0; i < s->n; i++) { t.r[i] = s->c[i]; t.c[i] = -s->r[i]; }
    normalize_raw(&t);
    return t;
}

static RawShape flip_h(RawShape *s) {
    RawShape t; t.n = s->n;
    for (int i = 0; i < s->n; i++) { t.r[i] = s->r[i]; t.c[i] = -s->c[i]; }
    normalize_raw(&t);
    return t;
}

static void build_orient(Orient *o, RawShape *s) {
    o->n = s->n;
    int maxr = 0, maxc = 0;
    for (int i = 0; i < s->n; i++) {
        o->dr[i] = s->r[i];
        o->dc[i] = s->c[i];
        if (s->r[i] > maxr) maxr = s->r[i];
        if (s->c[i] > maxc) maxc = s->c[i];
    }
    o->maxR = maxr;
    o->maxC = maxc;
}

static void compute_orientations(RawShape *base, ShapeSet *ss) {
    RawShape all[8];
    int num = 0;
    RawShape cur = *base;
    for (int f = 0; f < 2; f++) {
        for (int r = 0; r < 4; r++) {
            int dup = 0;
            for (int i = 0; i < num; i++)
                if (same_raw(&cur, &all[i])) { dup = 1; break; }
            if (!dup) all[num++] = cur;
            RawShape next = rotate90(&cur);
            cur = next;
        }
        RawShape flipped = flip_h(&cur);
        cur = flipped;
    }
    ss->num_orients = num;
    ss->num_cells = base->n;
    for (int i = 0; i < num; i++)
        build_orient(&ss->orients[i], &all[i]);
}

// ---- Grid and backtracking state ----

// grid: 0=empty, 1=occupied
static char grid[MAX_H][MAX_W];
static int gW, gH;

// Remaining counts of each shape type
static int remain[MAX_TYPES];
static int n_types;
static int total_remaining; // total pieces left

// Find first empty cell (row-major)
static int find_first_empty(int *out_r, int *out_c) {
    for (int r = 0; r < gH; r++)
        for (int c = 0; c < gW; c++)
            if (!grid[r][c]) { *out_r = r; *out_c = c; return 1; }
    return 0;
}

// Place piece: orient o of shape s, anchor at (pr, pc)
// Returns 1 if placed successfully, 0 if collision
static int place(int si, int oi, int pr, int pc) {
    Orient *o = &shapes[si].orients[oi];
    // Check all cells
    for (int k = 0; k < o->n; k++) {
        int r = pr + o->dr[k], c = pc + o->dc[k];
        if (r < 0 || r >= gH || c < 0 || c >= gW) return 0;
        if (grid[r][c]) return 0;
    }
    // Place
    for (int k = 0; k < o->n; k++)
        grid[pr + o->dr[k]][pc + o->dc[k]] = 1;
    return 1;
}

static void unplace(int si, int oi, int pr, int pc) {
    Orient *o = &shapes[si].orients[oi];
    for (int k = 0; k < o->n; k++)
        grid[pr + o->dr[k]][pc + o->dc[k]] = 0;
}

// Count empty cells reachable from (r,c) using flood fill (BFS-lite)
// Uses a small stack; returns count or -1 if > limit
static char visited[MAX_H][MAX_W];
static int bfs_queue[MAX_H * MAX_W * 2];

static int count_component(int r, int c, int limit) {
    int head = 0, tail = 0;
    bfs_queue[tail++] = r * MAX_W + c;
    visited[r][c] = 1;
    int count = 0;
    int dr[4] = {-1,1,0,0}, dc[4] = {0,0,-1,1};
    while (head < tail) {
        int pos = bfs_queue[head++];
        int cr = pos / MAX_W, cc = pos % MAX_W;
        count++;
        if (count > limit) {
            // clean up visited
            for (int i = 0; i < tail; i++) {
                visited[bfs_queue[i] / MAX_W][bfs_queue[i] % MAX_W] = 0;
            }
            return count;
        }
        for (int d = 0; d < 4; d++) {
            int nr = cr + dr[d], nc = cc + dc[d];
            if (nr >= 0 && nr < gH && nc >= 0 && nc < gW && !grid[nr][nc] && !visited[nr][nc]) {
                visited[nr][nc] = 1;
                bfs_queue[tail++] = nr * MAX_W + nc;
            }
        }
    }
    // clean up visited
    for (int i = 0; i < tail; i++)
        visited[bfs_queue[i] / MAX_W][bfs_queue[i] % MAX_W] = 0;
    return count;
}

// min cells per piece
static int min_piece_cells;

// Check if any empty connected component is too small
// Returns 1 if prunable (infeasible)
static int prune_connectivity(void) {
    // count total empty cells
    int empty = 0;
    for (int r = 0; r < gH; r++)
        for (int c = 0; c < gW; c++)
            if (!grid[r][c]) empty++;
    if (empty == 0 && total_remaining == 0) return 0; // done
    if (empty == 0 && total_remaining > 0) return 1; // no space
    
    // Check each component
    memset(visited, 0, sizeof(visited));
    // remaining cells needed
    int cells_needed = 0;
    for (int s = 0; s < n_types; s++)
        cells_needed += remain[s] * shapes[s].num_cells;
    
    if (empty < cells_needed) return 1; // impossible by area
    
    // check components
    // We need: each component size ≥ min_piece_cells (if any piece remains)
    if (total_remaining == 0) return 0;
    
    for (int r = 0; r < gH; r++) {
        for (int c = 0; c < gW; c++) {
            if (!grid[r][c] && !visited[r][c]) {
                int sz = count_component(r, c, cells_needed + 1);
                // mark all visited cells as visited
                // (count_component already cleared visited for us)
                // We need to re-mark for the outer loop
                // Actually we need a different approach - let me just use a simple flag
                // Let me redo: count_component clears visited. 
                // We need to mark visited for the outer loop to skip already-counted cells.
                // This creates an issue. Let me use a separate visited array.
                if (sz < min_piece_cells) return 1; // component too small
                // Mark this component in visited for outer loop
                // Actually we already cleared it. Let me just mark it now.
                // Re-run flood fill to mark...
                // This is inefficient. Let me use a different approach.
                (void)sz; // suppress warning
            }
        }
    }
    return 0;
}

// Better pruning: use a visited array that persists across calls
static char visited2[MAX_H][MAX_W];

static int prune_fast(void) {
    int cells_needed = 0;
    for (int s = 0; s < n_types; s++)
        cells_needed += remain[s] * shapes[s].num_cells;
    
    if (total_remaining == 0) return 0;
    
    // Count empty cells and check each component
    memset(visited2, 0, sizeof(visited2));
    
    int dr[4] = {-1,1,0,0}, dc[4] = {0,0,-1,1};
    
    for (int r = 0; r < gH; r++) {
        for (int c = 0; c < gW; c++) {
            if (!grid[r][c] && !visited2[r][c]) {
                // BFS from (r,c)
                int head = 0, tail = 0;
                bfs_queue[tail++] = r * MAX_W + c;
                visited2[r][c] = 1;
                int sz = 0;
                while (head < tail) {
                    int pos = bfs_queue[head++];
                    int cr = pos / MAX_W, cc = pos % MAX_W;
                    sz++;
                    for (int d = 0; d < 4; d++) {
                        int nr = cr + dr[d], nc = cc + dc[d];
                        if (nr >= 0 && nr < gH && nc >= 0 && nc < gW && !grid[nr][nc] && !visited2[nr][nc]) {
                            visited2[nr][nc] = 1;
                            bfs_queue[tail++] = nr * MAX_W + nc;
                        }
                    }
                }
                if (sz < min_piece_cells) return 1;
            }
        }
    }
    return 0;
}

// The main backtracking function
// Returns 1 if solution found, 0 if not
static int solve(void) {
    int fr, fc;
    if (!find_first_empty(&fr, &fc)) {
        // No empty cell - check if all pieces placed
        return (total_remaining == 0);
    }
    
    if (total_remaining == 0) {
        // Pieces all placed but empty cells remain - that's OK
        return 1;
    }
    
    // Try each shape type
    for (int si = 0; si < n_types; si++) {
        if (remain[si] == 0) continue;
        
        ShapeSet *ss = &shapes[si];
        
        // Try each orientation
        for (int oi = 0; oi < ss->num_orients; oi++) {
            Orient *o = &ss->orients[oi];
            
            // The piece must cover (fr, fc)
            // Try each cell of the piece as the cell covering (fr, fc)
            for (int k = 0; k < o->n; k++) {
                int pr = fr - o->dr[k];
                int pc = fc - o->dc[k];
                
                // Check bounds first
                if (pr < 0 || pr + o->maxR >= gH || pc < 0 || pc + o->maxC >= gW) continue;
                
                // Ensure that (fr, fc) is actually covered and pieces before it aren't
                // Actually we need: all cells (pr+dr[j], pc+dc[j]) >= (fr,fc) in row-major
                // for cells j < k (the cells before our anchor in the piece)
                // Wait no - we need that our piece covers (fr,fc) AND no cell of the piece
                // is BEFORE (fr,fc) in row-major order (since those would already be filled)
                // But that's not quite right either - some cells before (fr,fc) might be filled
                // by other pieces.
                // The correct check: all cells in this piece that come BEFORE (fr,fc) in
                // row-major must already be occupied. But since fr,fc is the FIRST empty,
                // if any cell of the piece is before (fr,fc) and empty, it's a contradiction.
                // Actually: if any cell of piece is BEFORE (fr,fc) in row-major order,
                // that cell must be occupied (it's before the first empty).
                // So: for each cell j of piece at (pr+dr[j], pc+dc[j]):
                //   if (pr+dr[j])*gW + (pc+dc[j]) < fr*gW + fc, then it must be occupied.
                //   But we're about to PLACE this piece (all cells empty), so this is
                //   only valid if those cells are already occupied by OTHER pieces.
                //   But the piece cells must all be empty for placement.
                //   Contradiction! So we should skip if any piece cell is before (fr,fc).
                
                int valid = 1;
                int fr_idx = fr * gW + fc;
                for (int j = 0; j < o->n; j++) {
                    if (j == k) continue;
                    int nr = pr + o->dr[j], nc = pc + o->dc[j];
                    int idx = nr * gW + nc;
                    if (idx < fr_idx) { valid = 0; break; } // cell before first empty
                }
                if (!valid) continue;
                
                // Try to place
                if (!place(si, oi, pr, pc)) continue;
                
                remain[si]--;
                total_remaining--;
                
                // Pruning
                if (!prune_fast()) {
                    if (solve()) {
                        unplace(si, oi, pr, pc);
                        remain[si]++;
                        total_remaining++;
                        return 1;
                    }
                }
                
                unplace(si, oi, pr, pc);
                remain[si]++;
                total_remaining++;
            }
        }
    }
    return 0;
}

// ---- Parsing ----

static void parse_input(const char *filename, int *answer) {
    FILE *f = fopen(filename, "r");
    if (!f) { fprintf(stderr, "Cannot open %s\n", filename); exit(1); }
    
    char line[512];
    num_shapes = 0;
    
    // Parse shapes
    // Read until blank line after last shape
    RawShape raw;
    int in_shape = 0;
    int current_shape_idx = -1;
    
    while (fgets(line, sizeof(line), f)) {
        // Remove newline
        int len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = 0;
        
        if (len == 0) {
            // Blank line
            if (in_shape && current_shape_idx >= 0) {
                compute_orientations(&raw, &shapes[current_shape_idx]);
            }
            in_shape = 0;
            current_shape_idx = -1;
            continue;
        }
        
        // Check if it's a shape header: "N:"
        if (line[1] == ':' || (line[2] == ':')) {
            // Finish previous shape if any
            if (in_shape && current_shape_idx >= 0) {
                compute_orientations(&raw, &shapes[current_shape_idx]);
                in_shape = 0;
            }
            current_shape_idx = num_shapes++;
            raw.n = 0;
            in_shape = 1;
            // row counter will be tracked separately
            // We need to track which row we're on
            // Actually we need to know the row number within the shape
            // Let me use a different approach
            continue;
        }
        
        // Check if it's a region line: "NxM: c0 c1 ..."
        if (strchr(line, 'x') && strchr(line, ':')) {
            // Finish any pending shape
            if (in_shape && current_shape_idx >= 0) {
                compute_orientations(&raw, &shapes[current_shape_idx]);
                in_shape = 0;
                current_shape_idx = -1;
            }
            break; // switch to region parsing
        }
        
        // It's a shape row
        if (in_shape) {
            // Count current rows for this shape
            // We need to track row index - let me count from raw.n
            // Actually row = (number of rows added so far for this shape)
            // Let me find max row already in raw
            int row = 0;
            for (int i = 0; i < raw.n; i++) if (raw.r[i] >= row) row = raw.r[i] + 1;
            for (int col = 0; col < len; col++) {
                if (line[col] == '#') {
                    raw.r[raw.n] = row;
                    raw.c[raw.n] = col;
                    raw.n++;
                }
            }
        }
    }
    
    // Finish last shape if still open
    if (in_shape && current_shape_idx >= 0) {
        compute_orientations(&raw, &shapes[current_shape_idx]);
    }
    
    // Re-read file to find region lines
    rewind(f);
    
    // Skip to first region line
    while (fgets(line, sizeof(line), f)) {
        int len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = 0;
        if (len > 0 && strchr(line, 'x') && strchr(line, ':')) break;
    }
    
    // We already have the first region line in `line`
    int count = 0;
    do {
        int len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = 0;
        if (len == 0) continue;
        
        // Parse "WxH: c0 c1 c2 ..."
        int W, H;
        if (sscanf(line, "%dx%d:", &W, &H) != 2) continue;
        
        char *colon = strchr(line, ':');
        if (!colon) continue;
        
        int counts[MAX_TYPES] = {0};
        int nc = 0;
        char *p = colon + 1;
        while (*p) {
            while (*p == ' ') p++;
            if (*p == 0) break;
            counts[nc++] = atoi(p);
            while (*p && *p != ' ') p++;
        }
        
        // Compute min piece cells
        min_piece_cells = 999;
        int total_cells = 0;
        n_types = nc;
        total_remaining = 0;
        for (int i = 0; i < nc; i++) {
            remain[i] = counts[i];
            total_remaining += counts[i];
            if (counts[i] > 0 && shapes[i].num_cells < min_piece_cells)
                min_piece_cells = shapes[i].num_cells;
            total_cells += counts[i] * shapes[i].num_cells;
        }
        if (min_piece_cells == 999) min_piece_cells = 1;
        
        // Quick area check
        if (total_cells > W * H) {
            // Impossible
        } else {
            // Set up grid
            gW = W; gH = H;
            memset(grid, 0, sizeof(grid));
            
            // Solve
            if (solve()) count++;
        }
    } while (fgets(line, sizeof(line), f));
    
    fclose(f);
    *answer = count;
}

// Better parsing that handles multi-digit shape indices
static void parse_input_v2(const char *filename, int *answer) {
    FILE *f = fopen(filename, "r");
    if (!f) { fprintf(stderr, "Cannot open %s\n", filename); exit(1); }
    
    char line[1024];
    num_shapes = 0;
    
    // Phase 1: parse shapes
    RawShape raw_shapes[MAX_SHAPES];
    int num_raw = 0;
    int in_shape = 0;
    int cur_row = 0;
    
    while (fgets(line, sizeof(line), f)) {
        int len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = 0;
        
        if (len == 0) {
            // blank line
            if (in_shape) {
                // finish shape
                in_shape = 0;
            }
            continue;
        }
        
        // Shape header: starts with digits followed by ':'
        // Region line: "NxM: ..."
        // Shape row: starts with '#' or '.'
        
        if (line[0] == '#' || line[0] == '.') {
            if (!in_shape) {
                // Start new shape data - but we should have seen header
                // This shouldn't happen with well-formed input
            }
            // Parse shape row
            if (in_shape && num_raw > 0) {
                RawShape *rs = &raw_shapes[num_raw - 1];
                for (int col = 0; col < len; col++) {
                    if (line[col] == '#') {
                        if (rs->n < MAX_CELLS_PER_SHAPE) {
                            rs->r[rs->n] = cur_row;
                            rs->c[rs->n] = col;
                            rs->n++;
                        }
                    }
                }
            }
            cur_row++;
            continue;
        }
        
        // Check for shape header: "N:" or "NN:"
        {
            int idx = -1;
            int pos = 0;
            while (pos < len && line[pos] >= '0' && line[pos] <= '9') pos++;
            if (pos > 0 && pos < len && line[pos] == ':') {
                // shape header
                idx = atoi(line);
                if (in_shape) {
                    // finish previous - already done when we see next header
                }
                // Start new shape
                if (num_raw < MAX_SHAPES) {
                    raw_shapes[num_raw].n = 0;
                    num_raw++;
                }
                in_shape = 1;
                cur_row = 0;
                continue;
            }
        }
        
        // Region line or other
        // Stop reading shapes when we see a region line
        if (strchr(line, 'x') != NULL) {
            // might be region line
            int hascolon = (strchr(line, ':') != NULL);
            if (hascolon) {
                in_shape = 0;
                break;
            }
        }
        in_shape = 0;
    }
    
    // Build orientations for all shapes
    num_shapes = num_raw;
    for (int i = 0; i < num_raw; i++) {
        normalize_raw(&raw_shapes[i]);
        compute_orientations(&raw_shapes[i], &shapes[i]);
    }
    
    // Phase 2: parse regions
    // We're at the first region line in `line`
    int count = 0;
    
    do {
        int len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = 0;
        if (len == 0) continue;
        if (!strchr(line, 'x')) continue;
        if (!strchr(line, ':')) continue;
        // Check it's actually a region line (not shape header)
        // Region: "NxM: ..."
        int W = 0, H = 0;
        int ret = sscanf(line, "%dx%d:", &W, &H);
        if (ret != 2) continue;
        
        char *colon = strchr(line, ':');
        if (!colon) continue;
        
        int cnts[MAX_TYPES] = {0};
        int nc = 0;
        char *p = colon + 1;
        while (*p && nc < MAX_TYPES) {
            while (*p == ' ') p++;
            if (*p == 0) break;
            cnts[nc++] = atoi(p);
            while (*p && *p != ' ') p++;
        }
        
        // Compute needed cells
        int total_cells = 0;
        for (int i = 0; i < nc; i++)
            total_cells += cnts[i] * shapes[i].num_cells;
        
        int feasible = 0;
        if (total_cells <= W * H) {
            // Set up for solving
            min_piece_cells = 999;
            n_types = nc;
            total_remaining = 0;
            for (int i = 0; i < nc; i++) {
                remain[i] = cnts[i];
                total_remaining += cnts[i];
                if (cnts[i] > 0 && shapes[i].num_cells < min_piece_cells)
                    min_piece_cells = shapes[i].num_cells;
            }
            if (min_piece_cells == 999) min_piece_cells = 1;
            
            gW = W; gH = H;
            memset(grid, 0, sizeof(grid));
            memset(visited2, 0, sizeof(visited2));
            
            if (solve()) feasible = 1;
        }
        
        if (feasible) count++;
        
    } while (fgets(line, sizeof(line), f));
    
    fclose(f);
    *answer = count;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc > 1) part = atoi(argv[1]);
    
    int ans1 = 0;
    parse_input_v2("input.txt", &ans1);
    
    if (part == 0 || part == 1)
        printf("Part 1: %d\n", ans1);
    
    return 0;
}
