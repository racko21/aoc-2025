#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils/utils.h"

/* The input is a grid of rolls of paper ('@') and empty space ('.'). A roll
 * is accessible by a forklift if fewer than 4 of its up-to-8 neighboring
 * cells also contain a roll. Part 1 counts how many rolls are accessible. */

#define MAX_ROWS 1024

/* Splits buf in place on '\n', filling rows[] with pointers to each line and
 * returning the row count. Sets *width to the length of the first line. */
static int split_lines(char *buf, char *rows[], int *width) {
    int count = 0;
    char *line_start = buf;
    for (char *p = buf; ; p++) {
        if (*p == '\n' || *p == '\0') {
            int len = (int)(p - line_start);
            if (len > 0) {
                rows[count++] = line_start;
                if (count == 1) {
                    *width = len;
                }
            }
            if (*p == '\0') {
                break;
            }
            *p = '\0';
            line_start = p + 1;
        }
    }
    return count;
}

/* Counts how many of the 8 cells surrounding (r, c) currently hold a '@'. */
static int count_neighbors(char *rows[], int height, int width, int r, int c) {
    int neighbors = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) {
                continue;
            }
            int nr = r + dr;
            int nc = c + dc;
            if (nr < 0 || nr >= height || nc < 0 || nc >= width) {
                continue;
            }
            if (rows[nr][nc] == '@') {
                neighbors++;
            }
        }
    }
    return neighbors;
}

/* Counts '@' rolls of paper that have fewer than 4 '@' neighbors among the
 * 8 surrounding cells. */
long long solve_part1(const char *path) {
    char *buf = read_file(path);
    char *rows[MAX_ROWS];
    int width = 0;
    int height = split_lines(buf, rows, &width);

    long long accessible = 0;
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            if (rows[r][c] != '@') {
                continue;
            }
            if (count_neighbors(rows, height, width, r, c) < 4) {
                accessible++;
            }
        }
    }

    free(buf);
    return accessible;
}

/* Repeatedly removes, in waves, every '@' that currently has fewer than 4
 * '@' neighbors, until a wave removes none. Removals within a wave are
 * based on the grid state at the start of that wave, so a roll's neighbor
 * count is unaffected by other rolls removed in the same wave. Returns the
 * total number of rolls removed across all waves. */
long long solve_part2(const char *path) {
    char *buf = read_file(path);
    char *rows[MAX_ROWS];
    int width = 0;
    int height = split_lines(buf, rows, &width);

    char *removable = malloc((size_t)height * (size_t)width);
    if (removable == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }

    long long total_removed = 0;
    for (;;) {
        int found = 0;
        for (int r = 0; r < height; r++) {
            for (int c = 0; c < width; c++) {
                int idx = r * width + c;
                if (rows[r][c] == '@' && count_neighbors(rows, height, width, r, c) < 4) {
                    removable[idx] = 1;
                    found = 1;
                } else {
                    removable[idx] = 0;
                }
            }
        }
        if (!found) {
            break;
        }
        for (int r = 0; r < height; r++) {
            for (int c = 0; c < width; c++) {
                if (removable[r * width + c]) {
                    rows[r][c] = '.';
                    total_removed++;
                }
            }
        }
    }

    free(removable);
    free(buf);
    return total_removed;
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
        long long ans1 = solve_part1("day04/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("day04/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }
    return 0;
}
#endif
