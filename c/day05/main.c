/* Day 5: Cafeteria. The database lists fresh-ingredient ID ranges
 * (inclusive, possibly overlapping) followed by a list of available
 * ingredient IDs. Part 1 counts how many available IDs fall inside any
 * range. Part 2 ignores the available IDs and instead counts the total
 * number of distinct IDs covered by the ranges themselves. */
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils/utils.h"

typedef struct {
    long long start;
    long long end;
} Range;

static int cmp_range_start(const void *a, const void *b) {
    long long sa = ((const Range *)a)->start;
    long long sb = ((const Range *)b)->start;
    if (sa < sb) return -1;
    if (sa > sb) return 1;
    return 0;
}

/* Reads the database file, splitting lines into ranges ("N-M") and plain
 * IDs. Caller must free *ranges_out and *ids_out. */
static void parse_database(const char *path, Range **ranges_out, int *nranges_out,
                            long long **ids_out, int *nids_out) {
    char *text = read_file(path);

    int range_cap = 16, range_n = 0;
    Range *ranges = malloc((size_t)range_cap * sizeof(Range));

    int id_cap = 16, id_n = 0;
    long long *ids = malloc((size_t)id_cap * sizeof(long long));

    char *line = text;
    while (line != NULL && *line != '\0') {
        char *newline = strchr(line, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }

        if (line[0] != '\0') {
            char *dash = strchr(line, '-');
            if (dash != NULL) {
                long long start, end;
                if (sscanf(line, "%lld-%lld", &start, &end) == 2) {
                    if (range_n == range_cap) {
                        range_cap *= 2;
                        ranges = realloc(ranges, (size_t)range_cap * sizeof(Range));
                    }
                    ranges[range_n].start = start;
                    ranges[range_n].end = end;
                    range_n++;
                }
            } else {
                long long id = strtoll(line, NULL, 10);
                if (id_n == id_cap) {
                    id_cap *= 2;
                    ids = realloc(ids, (size_t)id_cap * sizeof(long long));
                }
                ids[id_n++] = id;
            }
        }

        line = (newline != NULL) ? newline + 1 : NULL;
    }

    free(text);

    *ranges_out = ranges;
    *nranges_out = range_n;
    *ids_out = ids;
    *nids_out = id_n;
}

/* Sorts ranges by start and merges overlapping ones in place. Returns the
 * number of merged ranges. */
static int merge_ranges(Range *ranges, int n) {
    if (n == 0) return 0;

    qsort(ranges, (size_t)n, sizeof(Range), cmp_range_start);

    int merged_n = 1;
    for (int i = 1; i < n; i++) {
        Range *top = &ranges[merged_n - 1];
        if (ranges[i].start <= top->end) {
            if (ranges[i].end > top->end) {
                top->end = ranges[i].end;
            }
        } else {
            ranges[merged_n++] = ranges[i];
        }
    }

    return merged_n;
}

/* Binary search over merged, sorted ranges for membership of id. */
static int id_in_ranges(const Range *merged, int merged_n, long long id) {
    int lo = 0, hi = merged_n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (id < merged[mid].start) {
            hi = mid - 1;
        } else if (id > merged[mid].end) {
            lo = mid + 1;
        } else {
            return 1;
        }
    }
    return 0;
}

long long solve_part1(const char *path) {
    Range *ranges;
    int nranges;
    long long *ids;
    int nids;
    parse_database(path, &ranges, &nranges, &ids, &nids);

    int merged_n = merge_ranges(ranges, nranges);

    long long fresh_count = 0;
    for (int i = 0; i < nids; i++) {
        if (id_in_ranges(ranges, merged_n, ids[i])) {
            fresh_count++;
        }
    }

    free(ranges);
    free(ids);
    return fresh_count;
}

long long solve_part2(const char *path) {
    Range *ranges;
    int nranges;
    long long *ids;
    int nids;
    parse_database(path, &ranges, &nranges, &ids, &nids);

    int merged_n = merge_ranges(ranges, nranges);

    long long total = 0;
    for (int i = 0; i < merged_n; i++) {
        total += ranges[i].end - ranges[i].start + 1;
    }

    free(ranges);
    free(ids);
    return total;
}

#ifndef UNIT_TEST
static double elapsed_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000.0 + (b.tv_nsec - a.tv_nsec) / 1e6;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc > 1) part = atoi(argv[1]);

    struct timespec t0, t1;
    if (part == 0 || part == 1) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans1 = solve_part1("day05/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("day05/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }
    return 0;
}
#endif
