#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXROWS 300
#define MAXCOLS 300

char grid[MAXROWS][MAXCOLS+1];
int rows, cols;

// splitter_hit[r][c] = 1 if the splitter at (r,c) has been activated
int splitter_hit[MAXROWS][MAXCOLS];

typedef struct {
    int row, col;
} Beam;

// Queue for BFS of beam starting positions
Beam queue[MAXROWS * MAXCOLS * 4];
int qhead, qtail;

// visited[r][c] = 1 if a beam starting at (r,c) has been processed
int visited[MAXROWS][MAXCOLS];

// next_split[r][c] = row of next ^ at column c at or below row r, -1 if none
int next_split[MAXROWS+1][MAXCOLS];

void precompute_next_split() {
    for (int c = 0; c < cols; c++) {
        next_split[rows][c] = -1;
        for (int r = rows-1; r >= 0; r--) {
            if (grid[r][c] == '^') {
                next_split[r][c] = r;
            } else {
                next_split[r][c] = next_split[r+1][c];
            }
        }
    }
}

int find_start(int *s_row, int *s_col) {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (grid[r][c] == 'S') {
                *s_row = r; *s_col = c;
                return 1;
            }
        }
    }
    return 0;
}

// Part 1: Simulate beams, count unique splitters activated
// Each beam starts at (row, col) moving downward.
// When it hits a splitter, that splitter is activated (count if first time).
// Spawn beams at (hit_row, c-1) and (hit_row, c+1) if not yet visited.
long long simulate_part1(int s_row, int s_col) {
    memset(splitter_hit, 0, sizeof(splitter_hit));
    memset(visited, 0, sizeof(visited));
    qhead = qtail = 0;
    
    queue[qtail++] = (Beam){s_row, s_col};
    visited[s_row][s_col] = 1;
    
    long long splits = 0;
    
    while (qhead < qtail) {
        Beam b = queue[qhead++];
        int r = b.row, c = b.col;
        
        int hit = next_split[r][c];
        
        if (hit < 0) {
            // Beam exits, no split
            continue;
        }
        
        // Beam hits splitter at (hit, c)
        if (!splitter_hit[hit][c]) {
            splitter_hit[hit][c] = 1;
            splits++;
        }
        
        // Spawn left beam at (hit, c-1)
        int lc = c - 1;
        if (lc >= 0 && !visited[hit][lc]) {
            visited[hit][lc] = 1;
            queue[qtail++] = (Beam){hit, lc};
        }
        
        // Spawn right beam at (hit, c+1)
        int rc2 = c + 1;
        if (rc2 < cols && !visited[hit][rc2]) {
            visited[hit][rc2] = 1;
            queue[qtail++] = (Beam){hit, rc2};
        }
    }
    
    return splits;
}

// Part 2: Count timelines using memoization.
// memo[r][c] = number of timelines for a single particle starting at (r,c) going down.
// timelines(r,c) = 
//   if no splitter below from (r,c): 1 (exits, 1 timeline)
//   if splitter at (hit,c):
//     left contribution: (lc >= 0) ? memo[hit][lc] : 1  (exits immediately = 1)
//     right contribution: (rc2 < cols) ? memo[hit][rc2] : 1
//     total: left + right
//
// We need memoization because the same starting position can be reached from multiple paths.
// Use iterative DFS with explicit stack to avoid C stack overflow on large inputs.

long long memo[MAXROWS][MAXCOLS];
char computed[MAXROWS][MAXCOLS];

typedef struct { int r, c, phase; } Frame;
Frame dfs_stack[MAXROWS * MAXCOLS * 2];

long long compute_timelines(int s_row, int s_col) {
    memset(computed, 0, sizeof(computed));
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            memo[r][c] = -1;
    
    int top = 0;
    dfs_stack[top++] = (Frame){s_row, s_col, 0};
    
    while (top > 0) {
        Frame *fr = &dfs_stack[top-1];
        int r = fr->r, c = fr->c;
        
        if (computed[r][c]) {
            top--;
            continue;
        }
        
        int hit = next_split[r][c];
        
        if (hit < 0) {
            // No splitter, beam exits = 1 timeline
            memo[r][c] = 1;
            computed[r][c] = 1;
            top--;
            continue;
        }
        
        int lc = c - 1, rc2 = c + 1;
        
        if (fr->phase == 0) {
            fr->phase = 1;
            // Push dependencies if not computed
            int pushed = 0;
            if (rc2 < cols && !computed[hit][rc2]) {
                dfs_stack[top++] = (Frame){hit, rc2, 0};
                pushed = 1;
            }
            if (lc >= 0 && !computed[hit][lc]) {
                dfs_stack[top++] = (Frame){hit, lc, 0};
                pushed = 1;
            }
            if (pushed) continue;
        }
        
        // Dependencies are ready
        long long left_t = (lc >= 0) ? memo[hit][lc] : 1;
        long long right_t = (rc2 < cols) ? memo[hit][rc2] : 1;
        
        memo[r][c] = left_t + right_t;
        computed[r][c] = 1;
        top--;
    }
    
    return memo[s_row][s_col];
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc > 1) part = atoi(argv[1]);
    
    FILE *f = fopen("input.txt", "r");
    if (!f) { fprintf(stderr, "Cannot open input.txt\n"); return 1; }
    
    rows = 0;
    while (fgets(grid[rows], MAXCOLS+1, f)) {
        int len = strlen(grid[rows]);
        while (len > 0 && (grid[rows][len-1] == '\n' || grid[rows][len-1] == '\r'))
            grid[rows][--len] = '\0';
        if (len > 0) {
            cols = len;
            rows++;
        }
    }
    fclose(f);
    
    precompute_next_split();
    
    int s_row = 0, s_col = 0;
    find_start(&s_row, &s_col);
    
    long long ans1 = 0, ans2 = 0;
    
    if (part == 0 || part == 1) {
        ans1 = simulate_part1(s_row, s_col);
        printf("Part 1: %lld\n", ans1);
    }
    
    if (part == 0 || part == 2) {
        ans2 = compute_timelines(s_row, s_col);
        printf("Part 2: %lld\n", ans2);
    }
    
    return 0;
}
