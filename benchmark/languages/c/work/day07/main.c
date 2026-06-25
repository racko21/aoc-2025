#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    int part = 0;
    if (argc > 1) part = atoi(argv[1]);

    FILE *f = fopen("input.txt", "r");
    if (!f) { perror("input.txt"); return 1; }

    char **grid = NULL;
    int rows = 0, cap = 0;
    int width = 0;
    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        int len = (int)strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = 0;
        if (len == 0) continue;
        if (rows >= cap) { cap = cap ? cap*2 : 16; grid = realloc(grid, cap*sizeof(char*)); }
        grid[rows] = malloc(len+1);
        memcpy(grid[rows], line, len+1);
        if (len > width) width = len;
        rows++;
    }
    fclose(f);

    // find S
    int scol = -1, srow = 0;
    for (int r = 0; r < rows && scol < 0; r++) {
        int len = (int)strlen(grid[r]);
        for (int c = 0; c < len; c++) if (grid[r][c] == 'S') { scol = c; srow = r; break; }
    }

    // helper inline
    #define AT(r,c) ( ((c) >= 0 && (c) < (int)strlen(grid[r])) ? grid[r][c] : '.' )

    // Part 1: simulate beams, count splits. Use boolean array of active columns.
    long long splits = 0;
    {
        char *active = calloc(width, 1);
        char *next = calloc(width, 1);
        active[scol] = 1;
        for (int r = srow+1; r < rows; r++) {
            memset(next, 0, width);
            for (int c = 0; c < width; c++) {
                if (!active[c]) continue;
                if (AT(r,c) == '^') {
                    splits++;
                    if (c-1 >= 0) next[c-1] = 1;
                    if (c+1 < width) next[c+1] = 1;
                } else {
                    next[c] = 1;
                }
            }
            char *t = active; active = next; next = t;
        }
        free(active); free(next);
    }

    // Part 2: count timelines = number of distinct paths.
    long long timelines = 0;
    {
        long long *active = calloc(width, sizeof(long long));
        long long *next = calloc(width, sizeof(long long));
        active[scol] = 1;
        for (int r = srow+1; r < rows; r++) {
            for (int c = 0; c < width; c++) next[c] = 0;
            for (int c = 0; c < width; c++) {
                if (active[c] == 0) continue;
                long long cnt = active[c];
                if (AT(r,c) == '^') {
                    if (c-1 >= 0) next[c-1] += cnt;
                    if (c+1 < width) next[c+1] += cnt;
                } else {
                    next[c] += cnt;
                }
            }
            long long *t = active; active = next; next = t;
        }
        // sum remaining beams = number of paths that exited
        for (int c = 0; c < width; c++) timelines += active[c];
        free(active); free(next);
    }

    if (part == 1) printf("Part 1: %lld\n", splits);
    else if (part == 2) printf("Part 2: %lld\n", timelines);
    else { printf("Part 1: %lld\n", splits); printf("Part 2: %lld\n", timelines); }

    for (int r = 0; r < rows; r++) free(grid[r]);
    free(grid);
    return 0;
    (void)srow;
}
