/* Day 2: Gift Shop
 * Given comma-separated ranges "lo-hi", find every "invalid" product ID in
 * any range and sum them. Part 1: an ID is invalid if it is some digit
 * sequence repeated exactly twice (e.g. 6464). Part 2: invalid if it is some
 * digit sequence repeated at least twice (e.g. 123123123). Ranges span up to
 * ~10 digits, far too wide to brute force, so candidates are generated
 * directly from the repeating digit pattern instead of scanning every ID.
 */

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils/utils.h"

typedef struct {
    long long lo;
    long long hi;
} Range;

static long long ipow10(int n) {
    long long r = 1;
    for (int i = 0; i < n; i++) {
        r *= 10;
    }
    return r;
}

static int numdigits(long long x) {
    int n = 0;
    while (x > 0) {
        n++;
        x /= 10;
    }
    return n;
}

static long long ceil_div(long long a, long long b) {
    return (a + b - 1) / b;
}

/* Parses "lo-hi,lo-hi,..." into a malloc'd array; sets *count. Caller frees. */
static Range *parse_ranges(const char *text, int *count) {
    int cap = 16;
    int n = 0;
    Range *ranges = malloc((size_t)cap * sizeof(Range));
    if (!ranges) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }

    const char *p = text;
    while (*p) {
        while (*p && (*p < '0' || *p > '9')) {
            p++;
        }
        if (!*p) {
            break;
        }
        char *end;
        long long lo = strtoll(p, &end, 10);
        p = end;
        while (*p == '-') {
            p++;
        }
        long long hi = strtoll(p, &end, 10);
        p = end;

        if (n == cap) {
            cap *= 2;
            ranges = realloc(ranges, (size_t)cap * sizeof(Range));
            if (!ranges) {
                fprintf(stderr, "error: out of memory\n");
                exit(1);
            }
        }
        ranges[n].lo = lo;
        ranges[n].hi = hi;
        n++;
    }

    *count = n;
    return ranges;
}

/* Sums every ID in [lo, hi] made of a digit sequence repeated exactly
 * twice, by generating candidates directly from the repeated half. */
static long long invalid_sum_part1(long long lo, long long hi) {
    long long total = 0;
    int max_len = numdigits(hi);

    for (int len = numdigits(lo); len <= max_len; len++) {
        if (len % 2 != 0) {
            continue;
        }
        int half = len / 2;

        long long len_lo = ipow10(len - 1);
        long long len_hi = ipow10(len) - 1;
        long long range_lo = lo > len_lo ? lo : len_lo;
        long long range_hi = hi < len_hi ? hi : len_hi;
        if (range_lo > range_hi) {
            continue;
        }

        long long rep = ipow10(half) + 1;
        long long d_min = (half == 1) ? 1 : ipow10(half - 1);
        long long d_max = ipow10(half) - 1;

        long long d_lo = ceil_div(range_lo, rep);
        if (d_lo < d_min) {
            d_lo = d_min;
        }
        long long d_hi = range_hi / rep;
        if (d_hi > d_max) {
            d_hi = d_max;
        }

        for (long long d = d_lo; d <= d_hi; d++) {
            total += d * rep;
        }
    }
    return total;
}

static int cmp_ll(const void *a, const void *b) {
    long long x = *(const long long *)a;
    long long y = *(const long long *)b;
    return (x > y) - (x < y);
}

/* Sums every ID in [lo, hi] made of a digit sequence repeated at least
 * twice. For each candidate length, every proper divisor of the length is a
 * possible period; the same ID can be produced by more than one period
 * (e.g. 1111 via period 1 and period 2), so candidates are deduplicated
 * before summing. */
static long long invalid_sum_part2(long long lo, long long hi) {
    long long total = 0;
    int max_len = numdigits(hi);

    long long *cand = NULL;
    int cand_cap = 0;

    for (int len = numdigits(lo); len <= max_len; len++) {
        if (len < 2) {
            continue;
        }

        long long len_lo = ipow10(len - 1);
        long long len_hi = ipow10(len) - 1;
        long long range_lo = lo > len_lo ? lo : len_lo;
        long long range_hi = hi < len_hi ? hi : len_hi;
        if (range_lo > range_hi) {
            continue;
        }

        int cand_n = 0;

        for (int period = 1; period < len; period++) {
            if (len % period != 0) {
                continue;
            }
            int reps = len / period;

            long long base = ipow10(period);
            long long rep = 0;
            long long term = 1;
            for (int i = 0; i < reps; i++) {
                rep += term;
                term *= base;
            }

            long long d_min = (period == 1) ? 1 : ipow10(period - 1);
            long long d_max = ipow10(period) - 1;

            long long d_lo = ceil_div(range_lo, rep);
            if (d_lo < d_min) {
                d_lo = d_min;
            }
            long long d_hi = range_hi / rep;
            if (d_hi > d_max) {
                d_hi = d_max;
            }

            for (long long d = d_lo; d <= d_hi; d++) {
                if (cand_n == cand_cap) {
                    cand_cap = cand_cap == 0 ? 256 : cand_cap * 2;
                    cand = realloc(cand, (size_t)cand_cap * sizeof(long long));
                    if (!cand) {
                        fprintf(stderr, "error: out of memory\n");
                        exit(1);
                    }
                }
                cand[cand_n++] = d * rep;
            }
        }

        qsort(cand, (size_t)cand_n, sizeof(long long), cmp_ll);
        for (int i = 0; i < cand_n; i++) {
            if (i == 0 || cand[i] != cand[i - 1]) {
                total += cand[i];
            }
        }
    }

    free(cand);
    return total;
}

long long solve_part1(const char *path) {
    char *text = read_file(path);
    int count;
    Range *ranges = parse_ranges(text, &count);

    long long total = 0;
    for (int i = 0; i < count; i++) {
        total += invalid_sum_part1(ranges[i].lo, ranges[i].hi);
    }

    free(ranges);
    free(text);
    return total;
}

long long solve_part2(const char *path) {
    char *text = read_file(path);
    int count;
    Range *ranges = parse_ranges(text, &count);

    long long total = 0;
    for (int i = 0; i < count; i++) {
        total += invalid_sum_part2(ranges[i].lo, ranges[i].hi);
    }

    free(ranges);
    free(text);
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
        long long ans1 = solve_part1("day02/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("day02/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }
    return 0;
}
#endif
