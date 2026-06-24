#include <assert.h>
#include <stdio.h>

long long solve_part1(const char *path);
long long solve_part2(const char *path);

int main(void) {
    long long p1 = solve_part1("day02/example.txt");
    printf("Part 1 result: %lld\n", p1);
    assert(p1 == 1227775554);
    printf("Part 1 OK\n");

    long long p2 = solve_part2("day02/example.txt");
    printf("Part 2 result: %lld\n", p2);
    assert(p2 == 4174379265);
    printf("Part 2 OK\n");
    return 0;
}
