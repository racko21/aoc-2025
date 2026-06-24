#include <assert.h>
#include <stdio.h>

#include "utils/utils.h"

long long solve_part1(const char *path);
long long solve_part2(const char *path);

int main(void) {
    long long p1 = solve_part1("day07/example.txt");
    printf("Part 1 result: %lld\n", p1);
    assert(p1 == 21);
    printf("Part 1 OK: %lld\n", p1);

    long long p2 = solve_part2("day07/example.txt");
    printf("Part 2 result: %lld\n", p2);
    assert(p2 == 40);
    printf("Part 2 OK: %lld\n", p2);
    return 0;
}
