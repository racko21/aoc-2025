#include <assert.h>
#include <stdio.h>

long long solve_part1(const char *path);
long long solve_part2(const char *path);

int main(void) {
    long long p1 = solve_part1("day09/example.txt");
    printf("Part 1 (example): %lld\n", p1);
    assert(p1 == 50);
    printf("Part 1 OK: %lld\n", p1);

    long long p2 = solve_part2("day09/example.txt");
    printf("Part 2 (example): %lld\n", p2);
    assert(p2 == 24);
    printf("Part 2 OK: %lld\n", p2);
    return 0;
}
