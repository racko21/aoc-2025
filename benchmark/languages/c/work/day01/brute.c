#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    FILE *f = fopen("input.txt", "r");
    if (!f) return 1;
    char line[256];
    long long part1 = 0, part2 = 0;
    int pos = 50;

    while (fgets(line, sizeof(line), f)) {
        char dir;
        int dist;
        if (sscanf(line, " %c%d", &dir, &dist) != 2) continue;

        // Brute force: step one click at a time
        for (int i = 0; i < dist; i++) {
            if (dir == 'L') {
                pos = (pos - 1 + 100) % 100;
            } else {
                pos = (pos + 1) % 100;
            }
            if (pos == 0) part2++;
        }
        if (pos == 0) part1++;
    }
    fclose(f);
    printf("Part 1: %lld\n", part1);
    printf("Part 2: %lld\n", part2);
    return 0;
}
