#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROWS 1024
#define MAX_COLS 1024

static char grid[MAX_ROWS][MAX_COLS];
static int rows, cols;

static int dx[] = {-1,-1,-1, 0, 0, 1, 1, 1};
static int dy[] = {-1, 0, 1,-1, 1,-1, 0, 1};

static int count_neighbors(int r, int c) {
    int cnt = 0;
    for (int d = 0; d < 8; d++) {
        int nr = r + dx[d];
        int nc = c + dy[d];
        if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && grid[nr][nc] == '@')
            cnt++;
    }
    return cnt;
}

static int is_accessible(int r, int c) {
    return grid[r][c] == '@' && count_neighbors(r, c) < 4;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc >= 2) part = atoi(argv[1]);

    FILE *f = fopen("input.txt", "r");
    if (!f) { fprintf(stderr, "Cannot open input.txt\n"); return 1; }

    rows = 0;
    cols = 0;
    char line[MAX_COLS + 10];
    while (fgets(line, sizeof(line), f)) {
        int len = (int)strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            len--;
        if (len == 0) continue;
        if (cols == 0) cols = len;
        memcpy(grid[rows], line, len);
        grid[rows][len] = '\0';
        rows++;
        if (rows >= MAX_ROWS) break;
    }
    fclose(f);

    // Part 1: count accessible rolls
    long long part1 = 0;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            if (is_accessible(r, c))
                part1++;

    // Part 2: repeatedly remove accessible rolls until none left
    // Use a queue-based BFS approach
    // We need a copy of grid for part2
    static char grid2[MAX_ROWS][MAX_COLS];
    for (int r = 0; r < rows; r++)
        memcpy(grid2[r], grid[r], cols + 1);

    // Use a boolean "in_queue" array and a queue
    // Queue stores (r,c) encoded as r*MAX_COLS+c
    static int in_queue[MAX_ROWS * MAX_COLS];
    int *queue = malloc(MAX_ROWS * MAX_COLS * sizeof(int));
    if (!queue) { fprintf(stderr, "OOM\n"); return 1; }

    memset(in_queue, 0, sizeof(in_queue));

    int qhead = 0, qtail = 0;

    // Helper: count neighbors in grid2
    // We'll inline this as a lambda-style check

    // Initialize queue with all accessible cells
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (grid2[r][c] == '@') {
                // count neighbors in grid2
                int cnt = 0;
                for (int d = 0; d < 8; d++) {
                    int nr = r + dx[d], nc = c + dy[d];
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && grid2[nr][nc] == '@')
                        cnt++;
                }
                if (cnt < 4) {
                    int idx = r * MAX_COLS + c;
                    if (!in_queue[idx]) {
                        in_queue[idx] = 1;
                        queue[qtail++] = idx;
                    }
                }
            }
        }
    }

    long long part2 = 0;
    while (qhead < qtail) {
        int idx = queue[qhead++];
        int r = idx / MAX_COLS;
        int c = idx % MAX_COLS;
        in_queue[idx] = 0;

        // Check if still accessible in grid2
        if (grid2[r][c] != '@') continue;
        int cnt = 0;
        for (int d = 0; d < 8; d++) {
            int nr = r + dx[d], nc = c + dy[d];
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && grid2[nr][nc] == '@')
                cnt++;
        }
        if (cnt >= 4) continue; // not accessible anymore (shouldn't happen since we enqueue when accessible)

        // Remove this roll
        grid2[r][c] = '.';
        part2++;

        // Check neighbors: they might now be accessible
        for (int d = 0; d < 8; d++) {
            int nr = r + dx[d], nc = c + dy[d];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
            if (grid2[nr][nc] != '@') continue;
            // Count its neighbors
            int ncnt = 0;
            for (int d2 = 0; d2 < 8; d2++) {
                int nnr = nr + dx[d2], nnc = nc + dy[d2];
                if (nnr >= 0 && nnr < rows && nnc >= 0 && nnc < cols && grid2[nnr][nnc] == '@')
                    ncnt++;
            }
            if (ncnt < 4) {
                int nidx = nr * MAX_COLS + nc;
                if (!in_queue[nidx]) {
                    in_queue[nidx] = 1;
                    queue[qtail++] = nidx;
                }
            }
        }
    }

    free(queue);

    if (part == 1) {
        printf("Part 1: %lld\n", part1);
    } else if (part == 2) {
        printf("Part 2: %lld\n", part2);
    } else {
        printf("Part 1: %lld\n", part1);
        printf("Part 2: %lld\n", part2);
    }

    return 0;
}
