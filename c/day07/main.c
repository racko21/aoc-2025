#define _POSIX_C_SOURCE 199309L
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils/utils.h"

/* A tachyon beam enters at 'S' in the top row and travels straight down.
 * When it reaches a '^' splitter it stops there and two new beams continue
 * from the columns immediately left and right of the splitter. Beams that
 * land on the same column merge into a single beam. Part 1 counts how many
 * splitters are actually hit by a beam. */

#define MAX_LINES 4096

/* Splits buf in place on '\n', filling lines[] with pointers to each line
 * and returning the number of lines found. */
static int split_lines(char *buf, char *lines[], int max_lines) {
    int n = 0;
    char *p = buf;
    while (*p != '\0' && n < max_lines) {
        lines[n++] = p;
        char *nl = strchr(p, '\n');
        if (nl == NULL) {
            break;
        }
        *nl = '\0';
        p = nl + 1;
    }
    return n;
}

long long solve_part1(const char *path) {
    char *buf = read_file(path);
    char *lines[MAX_LINES];
    int nlines = split_lines(buf, lines, MAX_LINES);

    /* Drop trailing empty lines (e.g. from a final newline). */
    while (nlines > 0 && lines[nlines - 1][0] == '\0') {
        nlines--;
    }

    size_t width = strlen(lines[0]);
    bool *active = calloc(width, sizeof(bool));
    bool *next = calloc(width, sizeof(bool));
    if (active == NULL || next == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }

    for (size_t c = 0; c < width; c++) {
        if (lines[0][c] == 'S') {
            active[c] = true;
            break;
        }
    }

    long long count = 0;
    for (int r = 1; r < nlines; r++) {
        memset(next, 0, width * sizeof(bool));
        for (size_t c = 0; c < width; c++) {
            if (!active[c]) {
                continue;
            }
            if (lines[r][c] == '^') {
                count++;
                if (c > 0) {
                    next[c - 1] = true;
                }
                if (c + 1 < width) {
                    next[c + 1] = true;
                }
            } else {
                next[c] = true;
            }
        }
        bool *tmp = active;
        active = next;
        next = tmp;
    }

    free(active);
    free(next);
    free(buf);
    return count;
}

long long solve_part2(const char *path) {
    char *buf = read_file(path);
    char *lines[MAX_LINES];
    int nlines = split_lines(buf, lines, MAX_LINES);

    while (nlines > 0 && lines[nlines - 1][0] == '\0') {
        nlines--;
    }

    size_t width = strlen(lines[0]);
    long long *active = calloc(width, sizeof(long long));
    long long *next = calloc(width, sizeof(long long));
    if (active == NULL || next == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }

    for (size_t c = 0; c < width; c++) {
        if (lines[0][c] == 'S') {
            active[c] = 1;
            break;
        }
    }

    for (int r = 1; r < nlines; r++) {
        memset(next, 0, width * sizeof(long long));
        for (size_t c = 0; c < width; c++) {
            if (active[c] == 0) {
                continue;
            }
            if (lines[r][c] == '^') {
                if (c > 0) {
                    next[c - 1] += active[c];
                }
                if (c + 1 < width) {
                    next[c + 1] += active[c];
                }
            } else {
                next[c] += active[c];
            }
        }
        long long *tmp = active;
        active = next;
        next = tmp;
    }

    long long total = 0;
    for (size_t c = 0; c < width; c++) {
        total += active[c];
    }

    free(active);
    free(next);
    free(buf);
    return total;
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
        long long ans1 = solve_part1("day07/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("day07/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }
    return 0;
}
#endif
