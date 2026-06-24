#include <assert.h>
#include <stdio.h>

long long solve_part1(const char *path);
long long solve_part2(const char *path);

int main(void) {
    long long p1 = solve_part1("day11/example.txt");
    fprintf(stderr, "Part 1: %lld\n", p1);
    assert(p1 == 5);
    printf("Part 1 OK: %lld\n", p1);

    long long p2 = solve_part2("day11/example2.txt");
    fprintf(stderr, "Part 2: %lld\n", p2);
    assert(p2 == 2);
    printf("Part 2 OK: %lld\n", p2);
    return 0;
}
