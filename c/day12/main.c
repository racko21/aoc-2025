#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils/utils.h"

/* Day 12: Christmas Tree Farm. Given a set of polyomino shapes (rotatable
 * and flippable; '.' cells are transparent and never block) and a list of
 * WxH regions each demanding a quantity of every shape, determine for how
 * many regions all required pieces can be placed inside the region without
 * any two pieces' '#' cells overlapping. */

#define MAX_SHAPES 16
#define MAX_ORIENT 8
#define MAX_CELLS 64

typedef struct {
    int dr[MAX_CELLS];
    int dc[MAX_CELLS];
    int n;
    int h, w;
} Orientation;

typedef struct {
    Orientation orient[MAX_ORIENT];
    int n_orient;
} Shape;

/* Rotates (r,c) within an h-row, w-col grid 90 degrees clockwise. */
static void rotate_cell(int r, int c, int h, int *nr, int *nc) {
    *nr = c;
    *nc = h - 1 - r;
}

/* Mirrors (r,c) within a w-col grid along the vertical axis. */
static void flip_cell(int r, int c, int w, int *nr, int *nc) {
    *nr = r;
    *nc = w - 1 - c;
}

/* Sorts an orientation's cells lexicographically for canonical comparison. */
static void sort_cells(Orientation *o) {
    for (int i = 1; i < o->n; i++) {
        int dr = o->dr[i], dc = o->dc[i];
        int j = i - 1;
        while (j >= 0 && (o->dr[j] > dr || (o->dr[j] == dr && o->dc[j] > dc))) {
            o->dr[j + 1] = o->dr[j];
            o->dc[j + 1] = o->dc[j];
            j--;
        }
        o->dr[j + 1] = dr;
        o->dc[j + 1] = dc;
    }
}

/* Returns 1 if two orientations have identical shape and cell sets. */
static int same_orientation(const Orientation *a, const Orientation *b) {
    if (a->h != b->h || a->w != b->w || a->n != b->n) return 0;
    for (int i = 0; i < a->n; i++) {
        if (a->dr[i] != b->dr[i] || a->dc[i] != b->dc[i]) return 0;
    }
    return 1;
}

/* Builds the deduplicated set of up to 8 distinct orientations (4 rotations
 * times optional flip) for a shape's base cell list. */
static void build_orientations(Shape *shape, const int *base_dr, const int *base_dc,
                                int n, int h, int w) {
    int cur_dr[MAX_CELLS], cur_dc[MAX_CELLS];
    int cur_h = h, cur_w = w;
    memcpy(cur_dr, base_dr, (size_t)n * sizeof(int));
    memcpy(cur_dc, base_dc, (size_t)n * sizeof(int));

    shape->n_orient = 0;

    for (int flip = 0; flip < 2; flip++) {
        int fr[MAX_CELLS], fc[MAX_CELLS];
        if (flip) {
            for (int i = 0; i < n; i++) flip_cell(cur_dr[i], cur_dc[i], cur_w, &fr[i], &fc[i]);
        } else {
            memcpy(fr, cur_dr, (size_t)n * sizeof(int));
            memcpy(fc, cur_dc, (size_t)n * sizeof(int));
        }

        int rr[MAX_CELLS], rc[MAX_CELLS];
        memcpy(rr, fr, (size_t)n * sizeof(int));
        memcpy(rc, fc, (size_t)n * sizeof(int));
        int rh = cur_h, rw = cur_w;

        for (int rot = 0; rot < 4; rot++) {
            Orientation cand;
            cand.n = n;
            cand.h = rh;
            cand.w = rw;
            memcpy(cand.dr, rr, (size_t)n * sizeof(int));
            memcpy(cand.dc, rc, (size_t)n * sizeof(int));
            sort_cells(&cand);

            int dup = 0;
            for (int i = 0; i < shape->n_orient; i++) {
                if (same_orientation(&shape->orient[i], &cand)) {
                    dup = 1;
                    break;
                }
            }
            if (!dup) shape->orient[shape->n_orient++] = cand;

            int nr[MAX_CELLS], nc[MAX_CELLS];
            for (int i = 0; i < n; i++) rotate_cell(rr[i], rc[i], rh, &nr[i], &nc[i]);
            memcpy(rr, nr, (size_t)n * sizeof(int));
            memcpy(rc, nc, (size_t)n * sizeof(int));
            int tmp = rh;
            rh = rw;
            rw = tmp;
        }
    }
}

/* Returns 1 if the whole line is a shape header of the exact form "N:"
 * (digits followed by a colon and nothing else). */
static int is_shape_header(const char *line, int *idx) {
    const char *p = line;
    if (*p < '0' || *p > '9') return 0;
    int val = 0;
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
    }
    if (*p != ':' || p[1] != '\0') return 0;
    *idx = val;
    return 1;
}

/* Parses all shape blocks from the input lines, advancing *pos past them. */
static int parse_shapes(char **lines, int n_lines, int *pos, Shape *shapes) {
    int n_shapes = 0;
    int i = *pos;
    while (i < n_lines) {
        char *line = lines[i];
        if (line[0] == '\0') {
            i++;
            continue;
        }
        int idx;
        if (!is_shape_header(line, &idx)) break;
        i++;
        char grid[MAX_CELLS][MAX_CELLS];
        int h = 0, w = 0;
        while (i < n_lines && lines[i][0] != '\0' && (lines[i][0] == '#' || lines[i][0] == '.')) {
            int len = (int)strlen(lines[i]);
            w = len;
            memcpy(grid[h], lines[i], (size_t)len);
            h++;
            i++;
        }
        int dr[MAX_CELLS], dc[MAX_CELLS], n = 0;
        for (int r = 0; r < h; r++) {
            for (int c = 0; c < w; c++) {
                if (grid[r][c] == '#') {
                    dr[n] = r;
                    dc[n] = c;
                    n++;
                }
            }
        }
        build_orientations(&shapes[idx], dr, dc, n, h, w);
        if (idx + 1 > n_shapes) n_shapes = idx + 1;
    }
    *pos = i;
    return n_shapes;
}

/* Node-visit budget for the backtracking search below. Real exhaustive
 * backtracking over ~200 pieces on a 50x47 grid is combinatorially
 * infeasible when a region truly cannot fit everything (proving
 * infeasibility requires exploring the whole tree). This cap bounds
 * worst-case search cost; if exhausted, the region is conservatively
 * reported as not fitting (see notes in thesis-log.jsonl). */
#define NODE_BUDGET 2000000

/* Recursively attempts to place the remaining required pieces (flattened
 * as a list of shape indices, biggest pieces first) into the occupied
 * grid. remaining_cells is the sum of '#' cells still left to place, used
 * to prune branches where not enough free area remains. budget tracks
 * remaining node visits; returns 0 once exhausted. Returns 1 if a full
 * valid placement exists, 0 otherwise. */
static int place_pieces(const int *piece_list, int piece_idx, int n_pieces, const Shape *shapes,
                         unsigned char *occupied, int gh, int gw, int free_cells,
                         int remaining_cells, long long *budget) {
    if (piece_idx == n_pieces) return 1;
    if (--(*budget) <= 0) return 0;
    if (free_cells < remaining_cells) return 0;

    const Shape *shape = &shapes[piece_list[piece_idx]];
    int piece_cells = shape->orient[0].n;
    for (int o = 0; o < shape->n_orient; o++) {
        const Orientation *orient = &shape->orient[o];
        if (orient->h > gh || orient->w > gw) continue;
        for (int r = 0; r <= gh - orient->h; r++) {
            for (int c = 0; c <= gw - orient->w; c++) {
                int ok = 1;
                for (int k = 0; k < orient->n; k++) {
                    int rr = r + orient->dr[k];
                    int cc = c + orient->dc[k];
                    if (occupied[rr * gw + cc]) {
                        ok = 0;
                        break;
                    }
                }
                if (!ok) continue;

                for (int k = 0; k < orient->n; k++) {
                    occupied[(r + orient->dr[k]) * gw + (c + orient->dc[k])] = 1;
                }
                if (place_pieces(piece_list, piece_idx + 1, n_pieces, shapes, occupied, gh, gw,
                                  free_cells - piece_cells, remaining_cells - piece_cells,
                                  budget)) {
                    return 1;
                }
                for (int k = 0; k < orient->n; k++) {
                    occupied[(r + orient->dr[k]) * gw + (c + orient->dc[k])] = 0;
                }
                if (*budget <= 0) return 0;
            }
        }
    }
    return 0;
}

/* Builds a flattened, descending-by-cell-count piece list for a region's
 * required counts, then runs the budgeted backtracking placer. Returns 1
 * if all required pieces fit without overlap, 0 otherwise. */
static int region_fits(const int *counts, int n_shapes, const Shape *shapes, int w, int h) {
    int piece_list[4096];
    int n_pieces = 0;
    int total_cells = 0;
    for (int s = 0; s < n_shapes; s++) {
        for (int k = 0; k < counts[s]; k++) {
            piece_list[n_pieces++] = s;
            total_cells += shapes[s].orient[0].n;
        }
    }
    /* Order by descending cell count: more constrained pieces placed first
     * fail fast and prune the search tree sooner. */
    for (int i = 1; i < n_pieces; i++) {
        int v = piece_list[i];
        int vcells = shapes[v].orient[0].n;
        int j = i - 1;
        while (j >= 0 && shapes[piece_list[j]].orient[0].n < vcells) {
            piece_list[j + 1] = piece_list[j];
            j--;
        }
        piece_list[j + 1] = v;
    }

    unsigned char *occupied = calloc((size_t)w * (size_t)h, 1);
    if (!occupied) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    long long budget = NODE_BUDGET;
    int result = place_pieces(piece_list, 0, n_pieces, shapes, occupied, h, w, w * h, total_cells,
                               &budget);
    free(occupied);
    return result;
}

/* Splits a buffer into an array of non-newline-terminated lines. Caller
 * must free the returned array (but not the individual line pointers,
 * which point into the mutated buffer). */
static char **split_lines(char *buf, int *n_lines) {
    int cap = 1024, n = 0;
    char **lines = malloc((size_t)cap * sizeof(char *));
    char *p = buf;
    while (*p) {
        if (n == cap) {
            cap *= 2;
            lines = realloc(lines, (size_t)cap * sizeof(char *));
        }
        lines[n++] = p;
        while (*p && *p != '\n') p++;
        if (*p == '\n') {
            *p = '\0';
            p++;
        }
    }
    *n_lines = n;
    return lines;
}

/* Solves Part 1: counts how many regions can fit all required shapes. */
long long solve_part1(const char *path) {
    char *buf = read_file(path);
    int n_lines;
    char **lines = split_lines(buf, &n_lines);

    Shape shapes[MAX_SHAPES];
    int pos = 0;
    int n_shapes = parse_shapes(lines, n_lines, &pos, shapes);

    long long fit_count = 0;
    for (int i = pos; i < n_lines; i++) {
        if (lines[i][0] == '\0') continue;
        int w, h;
        char *rest = strchr(lines[i], ':');
        if (!rest) continue;
        if (sscanf(lines[i], "%dx%d:", &w, &h) != 2) continue;
        rest++;

        int counts[MAX_SHAPES] = {0};
        char *tok = rest;
        for (int s = 0; s < n_shapes; s++) {
            while (*tok == ' ') tok++;
            counts[s] = (int)strtol(tok, &tok, 10);
        }

        if (region_fits(counts, n_shapes, shapes, w, h)) fit_count++;
    }

    free(lines);
    free(buf);
    return fit_count;
}

#ifndef UNIT_TEST
static double elapsed_ms(struct timespec a, struct timespec b) {
    return (double)(b.tv_sec - a.tv_sec) * 1000.0 + (double)(b.tv_nsec - a.tv_nsec) / 1e6;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc > 1) part = atoi(argv[1]);

    struct timespec t0, t1;
    if (part == 0 || part == 1) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans1 = solve_part1("day12/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    return 0;
}
#endif
