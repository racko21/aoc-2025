#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc >= 2) part = atoi(argv[1]);

    FILE *f = fopen("input.txt", "r");
    if (!f) { fprintf(stderr, "Cannot open input.txt\n"); return 1; }

    char line[256];
    long long part1 = 0, part2 = 0;
    int pos = 50; // dial starts at 50

    while (fgets(line, sizeof(line), f)) {
        char dir;
        int dist;
        if (sscanf(line, " %c%d", &dir, &dist) != 2) continue;

        // Part 2: count passes through 0 during this rotation
        // For L: we step from pos, going down (mod 100)
        //   Step k (1..dist) lands on (pos - k + 10000) % 100
        //   Hits 0 when (pos - k) % 100 == 0, i.e. k % 100 == pos % 100
        //   First hit: if pos == 0, first_k = 100; else first_k = pos
        // For R: we step from pos, going up (mod 100)
        //   Step k (1..dist) lands on (pos + k) % 100
        //   Hits 0 when (pos + k) % 100 == 0, i.e. k % 100 == (100 - pos) % 100
        //   First hit: if pos == 0, first_k = 100; else first_k = 100 - pos
        long long clicks_through_zero = 0;
        if (dir == 'L') {
            int first_k = (pos == 0) ? 100 : pos;
            if (dist >= first_k) {
                // count k = first_k, first_k+100, first_k+200, ... <= dist
                clicks_through_zero = (long long)(dist - first_k) / 100 + 1;
            }
        } else { // R
            int first_k = (pos == 0) ? 100 : (100 - pos);
            if (dist >= first_k) {
                clicks_through_zero = (long long)(dist - first_k) / 100 + 1;
            }
        }
        part2 += clicks_through_zero;

        // Compute new position
        if (dir == 'L') {
            pos = ((pos - dist) % 100 + 100) % 100;
        } else {
            pos = (pos + dist) % 100;
        }

        // Part 1: count if final position is 0
        if (pos == 0) {
            part1++;
        }
    }

    fclose(f);

    // Part 2 total = clicks through 0 during rotation (including landing on 0)
    // But wait: part2 as computed counts landing on 0 as well (since we count step k=dist too if it hits 0)
    // part1 counts only end-of-rotation zeros
    // The problem says part2 = total times dial points at 0 during any click
    // That includes the final click landing on 0 (which part1 counts)
    // So part2 already includes everything (both mid-rotation and end-of-rotation zeros)
    // Let's verify with example: expected part1=3, part2=6

    if (part == 0 || part == 1) {
        printf("Part 1: %lld\n", part1);
    }
    if (part == 0 || part == 2) {
        printf("Part 2: %lld\n", part2);
    }

    return 0;
}
