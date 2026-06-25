#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    long long x, y, z;
} Point;

typedef struct {
    double dist;
    int a, b;
} Edge;

// Union-Find
static int *parent;
static int *rank_arr;
static int *sz;

int find(int x) {
    while (parent[x] != x) {
        parent[x] = parent[parent[x]]; // path compression (halving)
        x = parent[x];
    }
    return x;
}

// Returns 1 if merged (were different), 0 if same
int unite(int a, int b) {
    a = find(a);
    b = find(b);
    if (a == b) return 0;
    if (rank_arr[a] < rank_arr[b]) { int t = a; a = b; b = t; }
    parent[b] = a;
    sz[a] += sz[b];
    if (rank_arr[a] == rank_arr[b]) rank_arr[a]++;
    return 1;
}

int cmp_edge(const void *p1, const void *p2) {
    const Edge *e1 = (const Edge *)p1;
    const Edge *e2 = (const Edge *)p2;
    if (e1->dist < e2->dist) return -1;
    if (e1->dist > e2->dist) return 1;
    return 0;
}

int cmp_int_desc(const void *a, const void *b) {
    return *(int*)b - *(int*)a;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc >= 2) part = atoi(argv[1]);

    FILE *f = fopen("input.txt", "r");
    if (!f) { fprintf(stderr, "Cannot open input.txt\n"); return 1; }

    Point *pts = NULL;
    int n = 0, cap = 0;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        // trim
        int len = (int)strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r' || line[len-1] == ' '))
            line[--len] = '\0';
        if (len == 0) continue;

        if (n == cap) {
            cap = cap ? cap * 2 : 64;
            pts = realloc(pts, cap * sizeof(Point));
        }
        sscanf(line, "%lld,%lld,%lld", &pts[n].x, &pts[n].y, &pts[n].z);
        n++;
    }
    fclose(f);

    // Build all edges
    long long num_edges = (long long)n * (n - 1) / 2;
    Edge *edges = malloc(num_edges * sizeof(Edge));
    long long ei = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double dx = (double)(pts[i].x - pts[j].x);
            double dy = (double)(pts[i].y - pts[j].y);
            double dz = (double)(pts[i].z - pts[j].z);
            edges[ei].dist = sqrt(dx*dx + dy*dy + dz*dz);
            edges[ei].a = i;
            edges[ei].b = j;
            ei++;
        }
    }

    // Sort edges by distance
    qsort(edges, num_edges, sizeof(Edge), cmp_edge);

    // Initialize Union-Find
    parent = malloc(n * sizeof(int));
    rank_arr = calloc(n, sizeof(int));
    sz = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        parent[i] = i;
        sz[i] = 1;
    }

    long long part1_ans = 0;
    long long part2_ans = 0;

    // Process edges for Part 1: first 1000 connections (edges processed, regardless of merge)
    // Wait: re-read problem. "Connect together the 1000 pairs of junction boxes which are closest together"
    // The problem says "connect the 1000 pairs", i.e., process the 1000 shortest edges (even if same component).
    // But the example says "making the ten shortest connections" — and the 4th pair (431,825,988 and 425,690,689)
    // was already in same circuit but "nothing happens" — so it still counts as one of the ten!
    // So we process the 1000 shortest EDGES (pairs), not 1000 merges.

    int connections_made = 0;    // count of edges processed (regardless of merge)
    int num_components = n;
    int part2_done = 0;

    for (long long i = 0; i < num_edges; i++) {
        if (connections_made < 1000 || !part2_done) {
            // process this edge
            int merged = unite(edges[i].a, edges[i].b);
            if (merged) {
                num_components--;
                if (num_components == 1 && !part2_done) {
                    // Part 2: product of X coordinates
                    part2_ans = pts[edges[i].a].x * pts[edges[i].b].x;
                    part2_done = 1;
                }
            }
            connections_made++;

            if (connections_made == 1000 && !part1_ans) {
                // Compute Part 1 answer: product of 3 largest component sizes
                // Collect all component sizes
                int *sizes = malloc(n * sizeof(int));
                int sc = 0;
                for (int j = 0; j < n; j++) {
                    if (find(j) == j) {
                        sizes[sc++] = sz[j];
                    }
                }
                qsort(sizes, sc, sizeof(int), cmp_int_desc);
                part1_ans = (long long)sizes[0] * sizes[1] * sizes[2];
                free(sizes);
            }

            if (connections_made >= 1000 && part2_done) break;
        }
    }

    // If part2 not done yet (all merged before processing all needed edges), handle
    // (shouldn't happen in normal cases)

    if (part == 1 || part == 0) {
        printf("Part 1: %lld\n", part1_ans);
    }
    if (part == 2 || part == 0) {
        printf("Part 2: %lld\n", part2_ans);
    }

    free(pts);
    free(edges);
    free(parent);
    free(rank_arr);
    free(sz);

    return 0;
}
