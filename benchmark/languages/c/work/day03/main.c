#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// pick k digits in order from s (length n) to maximize the resulting number
static long long maxPick(const char *s, int n, int k) {
    // greedy stack: result has k digits; we can drop n-k digits total
    char stack[256];
    int top = 0;
    int drops = n - k;
    for (int i = 0; i < n; i++) {
        char c = s[i];
        while (top > 0 && drops > 0 && stack[top-1] < c) {
            top--;
            drops--;
        }
        stack[top++] = c;
    }
    // result is first k of stack
    long long val = 0;
    for (int i = 0; i < k; i++) {
        val = val * 10 + (stack[i] - '0');
    }
    return val;
}

int main(int argc, char **argv) {
    int part = 0;
    if (argc > 1) part = atoi(argv[1]);

    FILE *f = fopen("input.txt", "r");
    if (!f) return 1;

    char line[1024];
    long long sum1 = 0, sum2 = 0;
    while (fgets(line, sizeof(line), f)) {
        int n = (int)strlen(line);
        while (n > 0 && (line[n-1] == '\n' || line[n-1] == '\r')) line[--n] = 0;
        if (n == 0) continue;
        if (n >= 2) sum1 += maxPick(line, n, 2);
        if (n >= 12) sum2 += maxPick(line, n, 12);
    }
    fclose(f);

    if (part == 1) printf("Part 1: %lld\n", sum1);
    else if (part == 2) printf("Part 2: %lld\n", sum2);
    else {
        printf("Part 1: %lld\n", sum1);
        printf("Part 2: %lld\n", sum2);
    }
    return 0;
}
