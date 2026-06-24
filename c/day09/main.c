#define _POSIX_C_SOURCE 199309L

/* Day 9: Movie Theater. Parse (x,y) red tile positions and find the two
 * tiles that, used as opposite corners, form the rectangle of largest
 * area: area = (|dx|+1) * (|dy|+1). Brute-force all pairs. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils/utils.h"

typedef struct {
    long long x, y;
} Point;

static Point *parse_points(const char *buf, int *count) {
    int cap = 16;
    Point *pts = malloc(sizeof(Point) * (size_t)cap);
    if (!pts) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    int n = 0;
    const char *p = buf;
    while (*p) {
        long long x, y;
        int consumed = 0;
        if (sscanf(p, "%lld,%lld%n", &x, &y, &consumed) == 2) {
            if (n == cap) {
                cap *= 2;
                pts = realloc(pts, sizeof(Point) * (size_t)cap);
                if (!pts) {
                    fprintf(stderr, "error: out of memory\n");
                    exit(1);
                }
            }
            pts[n].x = x;
            pts[n].y = y;
            n++;
            p += consumed;
        }
        while (*p && *p != '\n') {
            p++;
        }
        if (*p == '\n') {
            p++;
        }
    }
    *count = n;
    return pts;
}

long long solve_part1(const char *path) {
    char *buf = read_file(path);
    int n;
    Point *pts = parse_points(buf, &n);

    long long best = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            long long dx = pts[i].x - pts[j].x;
            if (dx < 0) {
                dx = -dx;
            }
            long long dy = pts[i].y - pts[j].y;
            if (dy < 0) {
                dy = -dy;
            }
            long long area = (dx + 1) * (dy + 1);
            if (area > best) {
                best = area;
            }
        }
    }

    free(pts);
    free(buf);
    return best;
}

/* A compressed grid axis segment: real coordinates [start, start+width). */
typedef struct {
    long long start, width;
} Seg;

static int cmp_ll(const void *a, const void *b) {
    long long x = *(const long long *)a;
    long long y = *(const long long *)b;
    return (x > y) - (x < y);
}

/* Sorts and deduplicates vals (n entries) in place, returns new count. */
static int unique_sorted(long long *vals, int n) {
    qsort(vals, (size_t)n, sizeof(long long), cmp_ll);
    int m = 0;
    for (int i = 0; i < n; i++) {
        if (i == 0 || vals[i] != vals[m - 1]) {
            vals[m++] = vals[i];
        }
    }
    return m;
}

/* Binary search for val in sorted arr[0..n). val is assumed present. */
static int find_index(const long long *arr, int n, long long val) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (arr[mid] == val) {
            return mid;
        }
        if (arr[mid] < val) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return -1;
}

/* The loop of red tiles, connected edge-to-edge, bounds a filled rectilinear
 * polygon (red + green tiles). Real coordinates run up to ~1e5, far too
 * large for a literal grid, so the ~500 vertex x/y coordinates are
 * coordinate-compressed into singleton columns/rows (one vertex value) and
 * gap columns/rows (the constant-status run between two consecutive vertex
 * values). Polygon edges are marked as boundary cells on this compressed
 * grid, a flood fill from a padded exterior corner classifies every
 * remaining cell as outside or enclosed (interior), and a 2D prefix sum
 * over real cell-area then answers "is rectangle R fully red/green?" in
 * O(1) per candidate pair of red tiles. */
long long solve_part2(const char *path) {
    char *buf = read_file(path);
    int n;
    Point *pts = parse_points(buf, &n);

    long long minX = pts[0].x, maxX = pts[0].x, minY = pts[0].y, maxY = pts[0].y;
    for (int i = 1; i < n; i++) {
        if (pts[i].x < minX) minX = pts[i].x;
        if (pts[i].x > maxX) maxX = pts[i].x;
        if (pts[i].y < minY) minY = pts[i].y;
        if (pts[i].y > maxY) maxY = pts[i].y;
    }

    long long *xs = malloc(sizeof(long long) * (size_t)(n + 2));
    long long *ys = malloc(sizeof(long long) * (size_t)(n + 2));
    for (int i = 0; i < n; i++) {
        xs[i] = pts[i].x;
        ys[i] = pts[i].y;
    }
    xs[n] = minX - 1;
    xs[n + 1] = maxX + 1;
    ys[n] = minY - 1;
    ys[n + 1] = maxY + 1;

    int m = unique_sorted(xs, n + 2);
    int k = unique_sorted(ys, n + 2);

    int cols = 2 * m - 1;
    int rows = 2 * k - 1;

    Seg *colSeg = malloc(sizeof(Seg) * (size_t)cols);
    Seg *rowSeg = malloc(sizeof(Seg) * (size_t)rows);
    for (int i = 0; i < m; i++) {
        colSeg[2 * i].start = xs[i];
        colSeg[2 * i].width = 1;
        if (i + 1 < m) {
            long long w = xs[i + 1] - xs[i] - 1;
            colSeg[2 * i + 1].start = xs[i] + 1;
            colSeg[2 * i + 1].width = w > 0 ? w : 0;
        }
    }
    for (int j = 0; j < k; j++) {
        rowSeg[2 * j].start = ys[j];
        rowSeg[2 * j].width = 1;
        if (j + 1 < k) {
            long long h = ys[j + 1] - ys[j] - 1;
            rowSeg[2 * j + 1].start = ys[j] + 1;
            rowSeg[2 * j + 1].width = h > 0 ? h : 0;
        }
    }

    unsigned char *boundary = calloc((size_t)rows * (size_t)cols, 1);
    if (!boundary) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        Point a = pts[i];
        Point b = pts[(i + 1) % n];
        if (a.x == b.x) {
            long long ylo = a.y < b.y ? a.y : b.y;
            long long yhi = a.y < b.y ? b.y : a.y;
            int colIdx = 2 * find_index(xs, m, a.x);
            int rowLo = 2 * find_index(ys, k, ylo);
            int rowHi = 2 * find_index(ys, k, yhi);
            for (int r = rowLo; r <= rowHi; r++) {
                boundary[(size_t)r * (size_t)cols + (size_t)colIdx] = 1;
            }
        } else {
            long long xlo = a.x < b.x ? a.x : b.x;
            long long xhi = a.x < b.x ? b.x : a.x;
            int rowIdx = 2 * find_index(ys, k, a.y);
            int colLo = 2 * find_index(xs, m, xlo);
            int colHi = 2 * find_index(xs, m, xhi);
            for (int c = colLo; c <= colHi; c++) {
                boundary[(size_t)rowIdx * (size_t)cols + (size_t)c] = 1;
            }
        }
    }

    unsigned char *outside = calloc((size_t)rows * (size_t)cols, 1);
    int *queue = malloc(sizeof(int) * (size_t)rows * (size_t)cols);
    if (!outside || !queue) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    int qHead = 0, qTail = 0;
    outside[0] = 1;
    queue[qTail++] = 0;
    while (qHead < qTail) {
        int cur = queue[qHead++];
        int r = cur / cols;
        int c = cur % cols;
        static const int dr[4] = {-1, 1, 0, 0};
        static const int dc[4] = {0, 0, -1, 1};
        for (int d = 0; d < 4; d++) {
            int nr = r + dr[d];
            int nc = c + dc[d];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) {
                continue;
            }
            size_t idx = (size_t)nr * (size_t)cols + (size_t)nc;
            if (outside[idx] || boundary[idx]) {
                continue;
            }
            outside[idx] = 1;
            queue[qTail++] = nr * cols + nc;
        }
    }
    free(queue);

    long long *prefix = calloc((size_t)(rows + 1) * (size_t)(cols + 1), sizeof(long long));
    if (!prefix) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            size_t idx = (size_t)r * (size_t)cols + (size_t)c;
            int allowed = boundary[idx] || !outside[idx];
            long long area = allowed ? rowSeg[r].width * colSeg[c].width : 0;
            long long above = prefix[(size_t)r * (size_t)(cols + 1) + (size_t)(c + 1)];
            long long left = prefix[(size_t)(r + 1) * (size_t)(cols + 1) + (size_t)c];
            long long diag = prefix[(size_t)r * (size_t)(cols + 1) + (size_t)c];
            prefix[(size_t)(r + 1) * (size_t)(cols + 1) + (size_t)(c + 1)] = above + left - diag + area;
        }
    }

    long long best = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            long long x1 = pts[i].x < pts[j].x ? pts[i].x : pts[j].x;
            long long x2 = pts[i].x < pts[j].x ? pts[j].x : pts[i].x;
            long long y1 = pts[i].y < pts[j].y ? pts[i].y : pts[j].y;
            long long y2 = pts[i].y < pts[j].y ? pts[j].y : pts[i].y;

            int c1 = 2 * find_index(xs, m, x1);
            int c2 = 2 * find_index(xs, m, x2);
            int r1 = 2 * find_index(ys, k, y1);
            int r2 = 2 * find_index(ys, k, y2);

            long long totalArea = (x2 - x1 + 1) * (y2 - y1 + 1);
            long long sum = prefix[(size_t)(r2 + 1) * (size_t)(cols + 1) + (size_t)(c2 + 1)]
                           - prefix[(size_t)r1 * (size_t)(cols + 1) + (size_t)(c2 + 1)]
                           - prefix[(size_t)(r2 + 1) * (size_t)(cols + 1) + (size_t)c1]
                           + prefix[(size_t)r1 * (size_t)(cols + 1) + (size_t)c1];

            if (sum == totalArea && totalArea > best) {
                best = totalArea;
            }
        }
    }

    free(prefix);
    free(outside);
    free(boundary);
    free(colSeg);
    free(rowSeg);
    free(xs);
    free(ys);
    free(pts);
    free(buf);
    return best;
}

#ifndef UNIT_TEST
static double elapsed_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000.0 + (b.tv_nsec - a.tv_nsec) / 1e6;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc > 1) {
        part = atoi(argv[1]);
    }

    struct timespec t0, t1;
    if (part == 0 || part == 1) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans1 = solve_part1("day09/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("day09/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }

    return 0;
}
#endif
