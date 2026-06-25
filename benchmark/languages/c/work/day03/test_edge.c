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
    // Test with exactly 2 digits
    const char *s1 = "98";
    char r[3];
    best_k_digits(s1, 2, 2, r);
    printf("'98' pick 2: '%s' (expect 98)\n", r);
    
    // Test with exactly 12 digits
    const char *s2 = "987654321111";
    char r2[13];
    best_k_digits(s2, 12, 12, r2);
    printf("'987654321111' pick 12: '%s' (expect 987654321111)\n", r2);
    
    return 0;
}
