#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    long long lo;
    long long hi;
} Range;

int cmp_range(const void *a, const void *b) {
    const Range *ra = (const Range *)a;
    const Range *rb = (const Range *)b;
    if (ra->lo < rb->lo) return -1;
    if (ra->lo > rb->lo) return  1;
    if (ra->hi < rb->hi) return -1;
    if (ra->hi > rb->hi) return  1;
    return 0;
}

int main(int argc, char *argv[]) {
    int do_part1 = 1, do_part2 = 1;
    if (argc >= 2) {
        if (argv[1][0] == '1') do_part2 = 0;
        else if (argv[1][0] == '2') do_part1 = 0;
    }

    FILE *f = fopen("input.txt", "r");
    if (!f) { fprintf(stderr, "Cannot open input.txt\n"); return 1; }

    // Read all lines
    char line[256];
    int in_ranges = 1;  // start in range section

    int ranges_cap = 64;
    int ranges_count = 0;
    Range *ranges = malloc(ranges_cap * sizeof(Range));

    int ids_cap = 64;
    int ids_count = 0;
    long long *ids = malloc(ids_cap * sizeof(long long));

    while (fgets(line, sizeof(line), f)) {
        // Trim newline
        int len = (int)strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        if (len == 0) {
            // blank line -> switch to IDs section
            in_ranges = 0;
            continue;
        }

        if (in_ranges) {
            // parse "lo-hi"
            long long lo, hi;
            if (sscanf(line, "%lld-%lld", &lo, &hi) == 2) {
                if (ranges_count == ranges_cap) {
                    ranges_cap *= 2;
                    ranges = realloc(ranges, ranges_cap * sizeof(Range));
                }
                ranges[ranges_count].lo = lo;
                ranges[ranges_count].hi = hi;
                ranges_count++;
            }
        } else {
            // parse ID
            long long id;
            if (sscanf(line, "%lld", &id) == 1) {
                if (ids_count == ids_cap) {
                    ids_cap *= 2;
                    ids = realloc(ids, ids_cap * sizeof(long long));
                }
                ids[ids_count++] = id;
            }
        }
    }
    fclose(f);

    // Sort ranges for efficient lookup and merging
    qsort(ranges, ranges_count, sizeof(Range), cmp_range);

    // --- Part 1: count how many IDs are in any range ---
    long long part1 = 0;
    for (int i = 0; i < ids_count; i++) {
        long long id = ids[i];
        // Binary search for a range that could contain id
        int lo_idx = 0, hi_idx = ranges_count - 1, found = 0;
        // Find the last range with lo <= id
        int best = -1;
        while (lo_idx <= hi_idx) {
            int mid = (lo_idx + hi_idx) / 2;
            if (ranges[mid].lo <= id) {
                best = mid;
                lo_idx = mid + 1;
            } else {
                hi_idx = mid - 1;
            }
        }
        if (best >= 0 && ranges[best].hi >= id) {
            found = 1;
        }
        // Also check if there's an overlapping range that starts after id
        // but that can't contain id if lo > id, so no need.
        // However there might be multiple ranges with lo <= id, best is the last one.
        // But wait: we only checked the last range with lo<=id. We should check all ranges
        // with lo<=id to see if any has hi>=id. Since ranges are sorted by lo,
        // we can just check up to 'best' — but any range before 'best' with hi>=id also works.
        // Actually the best approach: find all ranges with lo<=id, check max hi among them.
        // We can precompute prefix max hi, but simpler: just scan backward from best.
        if (!found && best >= 0) {
            // check earlier ranges (they have smaller lo, might have large hi)
            for (int j = best - 1; j >= 0; j--) {
                if (ranges[j].hi >= id) { found = 1; break; }
                // since ranges[j].lo <= ranges[best].lo <= id,
                // if ranges[j].hi < id, earlier ones might still have hi >= id
                // we must keep checking
            }
        }
        if (found) part1++;
    }

    // --- Part 2: merge ranges and count total IDs ---
    // Build merged ranges
    int merged_cap = ranges_count + 1;
    Range *merged = malloc(merged_cap * sizeof(Range));
    int merged_count = 0;

    for (int i = 0; i < ranges_count; i++) {
        if (merged_count == 0) {
            merged[merged_count++] = ranges[i];
        } else {
            Range *last = &merged[merged_count - 1];
            if (ranges[i].lo <= last->hi + 1) {
                // overlapping or adjacent: extend
                if (ranges[i].hi > last->hi) last->hi = ranges[i].hi;
            } else {
                merged[merged_count++] = ranges[i];
            }
        }
    }

    long long part2 = 0;
    for (int i = 0; i < merged_count; i++) {
        part2 += merged[i].hi - merged[i].lo + 1;
    }

    if (do_part1) printf("Part 1: %lld\n", part1);
    if (do_part2) printf("Part 2: %lld\n", part2);

    free(ranges);
    free(ids);
    free(merged);
    return 0;
}
