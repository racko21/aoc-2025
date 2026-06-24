#include <assert.h>
#include <stdio.h>

long long solve_part1(const char *path);

int main(void) {
    long long p1 = solve_part1("day12/example.txt");
    printf("Part 1: %lld\n", p1);
    assert(p1 == 2);
    printf("Part 1 OK: %lld\n", p1);
    return 0;
}
