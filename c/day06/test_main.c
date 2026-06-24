#include <assert.h>
#include <stdio.h>
#include "utils/utils.h"

long long solve_part1(const char *path);
long long solve_part2(const char *path);

int main(void) {
    long long p1 = solve_part1("day06/example.txt");
    assert(p1 == 4277556);
    printf("Part 1 OK: %lld\n", p1);

    long long p2 = solve_part2("day06/example.txt");
    assert(p2 == 3263827);
    printf("Part 2 OK: %lld\n", p2);

    return 0;
}
