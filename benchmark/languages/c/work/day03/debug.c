#include <stdio.h>
#include <string.h>

static void best_k_digits(const char *s, int n, int K, char *result) {
    int start = 0;
    for (int k = 0; k < K; k++) {
        int remaining = K - k - 1;
        int end = n - 1 - remaining;
        char best = '0';
        for (int i = start; i <= end; i++) {
            if (s[i] > best) best = s[i];
        }
        int chosen = start;
        for (int i = start; i <= end; i++) {
            if (s[i] == best) { chosen = i; break; }
        }
        result[k] = s[chosen];
        start = chosen + 1;
    }
    result[K] = '\0';
}

int main(void) {
    const char *tests[] = {
        "987654321111111",
        "811111111111119",
        "234234234234278",
        "818181911112111"
    };
    for (int t = 0; t < 4; t++) {
        const char *s = tests[t];
        int n = strlen(s);
        char r[13];
        best_k_digits(s, n, 12, r);
        printf("%s -> %s\n", s, r);
    }
    return 0;
}
