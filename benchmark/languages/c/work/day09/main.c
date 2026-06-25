#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long long ll;

static int cmp_int(const void *a, const void *b) {
    int x = *(int*)a, y = *(int*)b;
    return x < y ? -1 : x > y ? 1 : 0;
}

static int unique_arr(int *arr, int n) {
    if (n == 0) return 0;
    int k = 0;
    for (int i = 0; i < n; i++) {
        if (i == 0 || arr[i] != arr[i-1]) arr[k++] = arr[i];
    }
    return k;
}

static int find_idx(int *arr, int n, int val) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (arr[mid] == val) return mid;
        else if (arr[mid] < val) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

// Check if point (px, py) is inside or on boundary of rectilinear polygon.
// Polygon edges connect consecutive vertices (either horiz or vert).
// Uses ray casting to the LEFT; vertical edges use half-open interval [ymin, ymax).
static int is_valid(int px, int py, int *xs, int *ys, int n) {
    // Check boundary first
    for (int i = 0; i < n; i++) {
        int x1 = xs[i], y1 = ys[i];
        int x2 = xs[(i+1)%n], y2 = ys[(i+1)%n];
        if (y1 == y2) {
            // Horizontal edge
            int xmin = x1 < x2 ? x1 : x2;
            int xmax = x1 > x2 ? x1 : x2;
            if (py == y1 && px >= xmin && px <= xmax) return 1;
        } else {
            // Vertical edge at x = x1 (= x2)
            int ymin = y1 < y2 ? y1 : y2;
            int ymax = y1 > y2 ? y1 : y2;
            if (px == x1 && py >= ymin && py <= ymax) return 1;
        }
    }
    // Ray to the left: count vertical edges with x1 <= px crossing y=py
    // Use half-open interval [ymin, ymax) to avoid double-counting at vertices
    int crossings = 0;
    for (int i = 0; i < n; i++) {
        int x1 = xs[i], y1 = ys[i];
        int x2 = xs[(i+1)%n], y2 = ys[(i+1)%n];
        if (y1 != y2) {
            // Vertical edge at x = x1
            int ymin = y1 < y2 ? y1 : y2;
            int ymax = y1 > y2 ? y1 : y2;
            if (py >= ymin && py < ymax && x1 <= px) {
                crossings++;
            }
        }
        (void)x2; // suppress unused warning (x2 == x1 for vertical edge)
    }
    return crossings % 2 == 1;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc > 1) part = atoi(argv[1]);

    FILE *f = fopen("input.txt", "r");
    if (!f) { perror("input.txt"); return 1; }

    int *xs = malloc(10000 * sizeof(int));
    int *ys = malloc(10000 * sizeof(int));
    int n = 0;
    int x, y;
    while (fscanf(f, "%d,%d", &x, &y) == 2) {
        xs[n] = x; ys[n] = y; n++;
    }
    fclose(f);

    // ---- Part 1 ----
    // Find largest rectangle with two red tiles as opposite corners.
    // Area = (|dx|+1) * (|dy|+1). Check all O(N^2) pairs.
    ll best1 = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            ll dx = xs[j] - xs[i]; if (dx < 0) dx = -dx;
            ll dy = ys[j] - ys[i]; if (dy < 0) dy = -dy;
            ll area = (dx + 1) * (dy + 1);
            if (area > best1) best1 = area;
        }
    }

    if (part == 1) {
        printf("Part 1: %lld\n", best1);
        free(xs); free(ys);
        return 0;
    }

    // ---- Part 2 ----
    // The red tiles form a rectilinear polygon (consecutive tiles share row/col).
    // Valid tiles = boundary + interior. Rectangle must have red corners and all
    // tiles inside must be valid.
    //
    // Use coordinate compression:
    // Include each vertex coordinate and coordinate+1. Between two adjacent
    // compressed coordinates, the inside/outside status is uniform.
    // Build prefix sum of invalid compressed cells to quickly check rectangles.

    int *cxs = malloc(4 * n * sizeof(int));
    int *cys = malloc(4 * n * sizeof(int));
    int ncx = 0, ncy = 0;

    for (int i = 0; i < n; i++) {
        cxs[ncx++] = xs[i];
        cxs[ncx++] = xs[i] + 1;
        cys[ncy++] = ys[i];
        cys[ncy++] = ys[i] + 1;
    }
    qsort(cxs, ncx, sizeof(int), cmp_int);
    ncx = unique_arr(cxs, ncx);
    qsort(cys, ncy, sizeof(int), cmp_int);
    ncy = unique_arr(cys, ncy);

    // Build validity grid in compressed coordinates.
    // Each compressed cell (ci, cj) represents real tile at (cxs[ci], cys[cj]).
    int *valid = calloc(ncx * ncy, sizeof(int));

    for (int cj = 0; cj < ncy; cj++) {
        int py = cys[cj];
        for (int ci = 0; ci < ncx; ci++) {
            int px = cxs[ci];
            valid[cj * ncx + ci] = is_valid(px, py, xs, ys, n);
        }
    }

    // Find compressed indices for each red tile
    int *ci_red = malloc(n * sizeof(int));
    int *cj_red = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        ci_red[i] = find_idx(cxs, ncx, xs[i]);
        cj_red[i] = find_idx(cys, ncy, ys[i]);
    }

    // 2D prefix sum of invalid cells (1 = invalid, 0 = valid)
    // prefix[(j+1)*(ncx+1)+(i+1)] = sum over [0..i] x [0..j]
    ll *prefix = calloc((ll)(ncx + 1) * (ncy + 1), sizeof(ll));

    for (int cj = 0; cj < ncy; cj++) {
        for (int ci = 0; ci < ncx; ci++) {
            ll inv = 1 - valid[cj * ncx + ci];
            prefix[(ll)(cj+1)*(ncx+1) + (ci+1)] = inv
                + prefix[(ll)cj*(ncx+1) + (ci+1)]
                + prefix[(ll)(cj+1)*(ncx+1) + ci]
                - prefix[(ll)cj*(ncx+1) + ci];
        }
    }

    // Query: count invalid compressed cells in [ci1..ci2] x [cj1..cj2] (inclusive)
    #define QUERY(ci1, ci2, cj1, cj2) \
        (prefix[(ll)(cj2+1)*(ncx+1) + (ci2+1)] \
        - prefix[(ll)(cj1)*(ncx+1) + (ci2+1)] \
        - prefix[(ll)(cj2+1)*(ncx+1) + (ci1)] \
        + prefix[(ll)(cj1)*(ncx+1) + (ci1)])

    // Check all pairs of red tiles; if their compressed rectangle is all-valid,
    // compute the real area.
    ll best2 = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (xs[i] == xs[j] && ys[i] == ys[j]) continue;

            int ci1 = ci_red[i], ci2 = ci_red[j];
            if (ci1 > ci2) { int t = ci1; ci1 = ci2; ci2 = t; }
            int cj1 = cj_red[i], cj2 = cj_red[j];
            if (cj1 > cj2) { int t = cj1; cj1 = cj2; cj2 = t; }

            if (QUERY(ci1, ci2, cj1, cj2) == 0) {
                ll dx = xs[j] - xs[i]; if (dx < 0) dx = -dx;
                ll dy = ys[j] - ys[i]; if (dy < 0) dy = -dy;
                ll area = (dx + 1) * (dy + 1);
                if (area > best2) best2 = area;
            }
        }
    }

    free(xs); free(ys);
    free(cxs); free(cys);
    free(valid);
    free(prefix);
    free(ci_red); free(cj_red);

    if (part == 2) {
        printf("Part 2: %lld\n", best2);
        return 0;
    }

    printf("Part 1: %lld\n", best1);
    printf("Part 2: %lld\n", best2);

    return 0;
}
