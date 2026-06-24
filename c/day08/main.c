#define _POSIX_C_SOURCE 199309L

/* Day 8: Playground. Parse 3D junction box coordinates, connect the N
 * closest pairs (by Euclidean distance) using union-find, then multiply
 * the sizes of the 3 largest resulting circuits. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils/utils.h"

typedef struct {
    long long x, y, z;
} Point;

typedef struct {
    long long dist;
    int a, b;
} Edge;

static int *parent;
static int *size_of;

static int find(int x) {
    while (parent[x] != x) {
        parent[x] = parent[parent[x]];
        x = parent[x];
    }
    return x;
}

static void unite(int a, int b) {
    int ra = find(a);
    int rb = find(b);
    if (ra == rb) {
        return;
    }
    if (size_of[ra] < size_of[rb]) {
        int tmp = ra;
        ra = rb;
        rb = tmp;
    }
    parent[rb] = ra;
    size_of[ra] += size_of[rb];
}

static int edge_cmp(const void *pa, const void *pb) {
    const Edge *ea = pa;
    const Edge *eb = pb;
    if (ea->dist < eb->dist) {
        return -1;
    }
    if (ea->dist > eb->dist) {
        return 1;
    }
    return 0;
}

static int size_cmp_desc(const void *pa, const void *pb) {
    int sa = *(const int *)pa;
    int sb = *(const int *)pb;
    return sb - sa;
}

/* Parses points from a comma-separated "x,y,z" per line file. */
static Point *parse_points(const char *path, int *count) {
    char *buf = read_file(path);
    int cap = 16;
    int n = 0;
    Point *pts = malloc((size_t)cap * sizeof(Point));

    char *line = buf;
    while (*line != '\0') {
        char *newline = strchr(line, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }
        if (line[0] != '\0') {
            long long x, y, z;
            if (sscanf(line, "%lld,%lld,%lld", &x, &y, &z) == 3) {
                if (n == cap) {
                    cap *= 2;
                    pts = realloc(pts, (size_t)cap * sizeof(Point));
                }
                pts[n].x = x;
                pts[n].y = y;
                pts[n].z = z;
                n++;
            }
        }
        if (newline == NULL) {
            break;
        }
        line = newline + 1;
    }

    free(buf);
    *count = n;
    return pts;
}

/* Connects the `connections` closest pairs of points and returns the
 * product of the sizes of the 3 largest connected components. */
static long long solve(const char *path, int connections) {
    int n;
    Point *pts = parse_points(path, &n);

    long edge_count = (long)n * (n - 1) / 2;
    Edge *edges = malloc((size_t)edge_count * sizeof(Edge));
    long e = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            long long dx = pts[i].x - pts[j].x;
            long long dy = pts[i].y - pts[j].y;
            long long dz = pts[i].z - pts[j].z;
            edges[e].dist = dx * dx + dy * dy + dz * dz;
            edges[e].a = i;
            edges[e].b = j;
            e++;
        }
    }

    qsort(edges, (size_t)edge_count, sizeof(Edge), edge_cmp);

    parent = malloc((size_t)n * sizeof(int));
    size_of = malloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) {
        parent[i] = i;
        size_of[i] = 1;
    }

    int limit = connections;
    if (limit > edge_count) {
        limit = (int)edge_count;
    }
    for (int k = 0; k < limit; k++) {
        unite(edges[k].a, edges[k].b);
    }

    int *sizes = malloc((size_t)n * sizeof(int));
    int sc = 0;
    for (int i = 0; i < n; i++) {
        if (find(i) == i) {
            sizes[sc++] = size_of[i];
        }
    }
    qsort(sizes, (size_t)sc, sizeof(int), size_cmp_desc);

    long long product = 1;
    for (int i = 0; i < 3 && i < sc; i++) {
        product *= sizes[i];
    }

    free(edges);
    free(parent);
    free(size_of);
    free(sizes);
    free(pts);

    return product;
}

long long solve_part1(const char *path, int connections) {
    return solve(path, connections);
}

/* Connects the closest unconnected pairs of points, skipping pairs already
 * in the same circuit, until every point is in a single circuit. Returns
 * the product of the X coordinates of the two points joined by the final
 * connection that merges everything into one circuit. */
long long solve_part2(const char *path) {
    int n;
    Point *pts = parse_points(path, &n);

    long edge_count = (long)n * (n - 1) / 2;
    Edge *edges = malloc((size_t)edge_count * sizeof(Edge));
    long e = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            long long dx = pts[i].x - pts[j].x;
            long long dy = pts[i].y - pts[j].y;
            long long dz = pts[i].z - pts[j].z;
            edges[e].dist = dx * dx + dy * dy + dz * dz;
            edges[e].a = i;
            edges[e].b = j;
            e++;
        }
    }

    qsort(edges, (size_t)edge_count, sizeof(Edge), edge_cmp);

    parent = malloc((size_t)n * sizeof(int));
    size_of = malloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) {
        parent[i] = i;
        size_of[i] = 1;
    }

    int components = n;
    long long x_a = 0;
    long long x_b = 0;
    for (long k = 0; k < edge_count && components > 1; k++) {
        int ra = find(edges[k].a);
        int rb = find(edges[k].b);
        if (ra == rb) {
            continue;
        }
        unite(ra, rb);
        components--;
        x_a = pts[edges[k].a].x;
        x_b = pts[edges[k].b].x;
    }

    free(edges);
    free(parent);
    free(size_of);
    free(pts);

    return x_a * x_b;
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
        long long ans1 = solve_part1("day08/input.txt", 1000);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("day08/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }

    return 0;
}
#endif
