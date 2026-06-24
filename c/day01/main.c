/* Day 1: Secret Entrance
 * A dial (0-99) starts at 50. Each input line is a rotation: L or R
 * followed by a distance, applied circularly (mod 100). Part 1 asks how
 * many times the dial lands exactly on 0 after a rotation.
 */

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DIAL_SIZE 100
#define START_POS 50

long long solve_part1(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: could not open %s\n", path);
        exit(1);
    }

    int pos = START_POS;
    long long zero_count = 0;
    char dir;
    int dist;

    while (fscanf(f, " %c%d", &dir, &dist) == 2) {
        if (dir == 'L') {
            pos = ((pos - dist) % DIAL_SIZE + DIAL_SIZE) % DIAL_SIZE;
        } else if (dir == 'R') {
            pos = (pos + dist) % DIAL_SIZE;
        }
        if (pos == 0) {
            zero_count++;
        }
    }

    fclose(f);
    return zero_count;
}

/* Floor division for positive divisor b (e.g. b == 100). */
static long long floor_div(long long a, long long b) {
    long long q = a / b;
    long long r = a % b;
    if (r != 0 && r < 0) {
        q--;
    }
    return q;
}

/* Counts every click that lands on 0, including overshoots from a single
 * large rotation (e.g. R1000 passes 0 ten times before settling).
 * Tracks an unbounded "virtual" position and counts multiples of 100
 * crossed between the old and new virtual position for each rotation.
 */
long long solve_part2(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: could not open %s\n", path);
        exit(1);
    }

    long long vpos = START_POS;
    long long zero_count = 0;
    char dir;
    int dist;

    while (fscanf(f, " %c%d", &dir, &dist) == 2) {
        long long old_vpos = vpos;
        if (dir == 'L') {
            vpos -= dist;
            zero_count += floor_div(old_vpos - 1, DIAL_SIZE) - floor_div(vpos - 1, DIAL_SIZE);
        } else if (dir == 'R') {
            vpos += dist;
            zero_count += floor_div(vpos, DIAL_SIZE) - floor_div(old_vpos, DIAL_SIZE);
        }
    }

    fclose(f);
    return zero_count;
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
        long long ans1 = solve_part1("day01/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("day01/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }
    return 0;
}
#endif
