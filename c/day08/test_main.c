#include <assert.h>
#include <stdio.h>

long long solve_part1(const char *path, int connections);
long long solve_part2(const char *path);

int main(void) {
    long long p1 = solve_part1("day08/example.txt", 10);
    printf("Part 1 (example): %lld\n", p1);
    assert(p1 == 40);
    printf("Part 1 OK: %lld\n", p1);

    long long p2 = solve_part2("day08/example.txt");
    printf("Part 2 (example): %lld\n", p2);
    assert(p2 == 25272);
    printf("Part 2 OK: %lld\n", p2);
    return 0;
}
