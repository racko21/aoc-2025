#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils/utils.h"

#define PART2_DIGITS 12

/* Each line is a bank of single-digit batteries. For every bank, pick two
 * digits (first index strictly before second) to form the largest possible
 * 2-digit number, then sum that maximum across all banks. */

/* Returns the largest two-digit number formable from digits[0..n-1] by
 * picking an earlier digit as the tens place and a later digit as the
 * ones place. */
static int best_pair(const char *digits, int n) {
    int best_suffix = -1;
    int best = 0;
    for (int i = n - 1; i >= 0; i--) {
        int d = digits[i] - '0';
        if (best_suffix >= 0) {
            int val = d * 10 + best_suffix;
            if (val > best) {
                best = val;
            }
        }
        if (d > best_suffix) {
            best_suffix = d;
        }
    }
    return best;
}

/* Sums the best two-digit number across every line of the input file. */
long long solve_part1(const char *path) {
    char *buf = read_file(path);
    long long total = 0;
    char *line_start = buf;
    for (char *p = buf; ; p++) {
        if (*p == '\n' || *p == '\0') {
            int len = (int)(p - line_start);
            if (len >= 2) {
                total += best_pair(line_start, len);
            }
            if (*p == '\0') {
                break;
            }
            line_start = p + 1;
        }
    }
    free(buf);
    return total;
}

/* Returns the largest number formable by choosing exactly m digits from
 * digits[0..n-1] while preserving their relative order. Uses a monotonic
 * stack: a smaller digit is popped in favor of a later larger one as long
 * as enough digits remain afterward to still reach m selected digits. */
static long long best_subsequence(const char *digits, int n, int m) {
    char stack[64];
    int top = 0;
    for (int i = 0; i < n; i++) {
        int d = digits[i] - '0';
        int remaining = n - i;
        while (top > 0 && stack[top - 1] < d && (top - 1) + remaining >= m) {
            top--;
        }
        if (top < m) {
            stack[top++] = (char)d;
        }
    }
    long long value = 0;
    for (int i = 0; i < m; i++) {
        value = value * 10 + stack[i];
    }
    return value;
}

/* Sums the largest 12-digit joltage across every line of the input file. */
long long solve_part2(const char *path) {
    char *buf = read_file(path);
    long long total = 0;
    char *line_start = buf;
    for (char *p = buf; ; p++) {
        if (*p == '\n' || *p == '\0') {
            int len = (int)(p - line_start);
            if (len >= PART2_DIGITS) {
                total += best_subsequence(line_start, len, PART2_DIGITS);
            }
            if (*p == '\0') {
                break;
            }
            line_start = p + 1;
        }
    }
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
        long long ans1 = solve_part1("day03/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("day03/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }
    return 0;
}
#endif
