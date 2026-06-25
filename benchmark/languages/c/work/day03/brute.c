#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static long long best_greedy(const char *s, int n, int K) {
    long long val = 0;
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
        val = val * 10 + (s[chosen] - '0');
        start = chosen + 1;
    }
    return val;
}

static long long best_brute2(const char *s, int n) {
    long long best = 0;
    for (int i = 0; i < n-1; i++) {
        for (int j = i+1; j < n; j++) {
            long long v = (s[i]-'0')*10 + (s[j]-'0');
            if (v > best) best = v;
        }
    }
    return best;
}

static long long best_brute_k(const char *s, int n, int K, int *picks, int depth, int start) {
    if (depth == K) {
        long long v = 0;
        for (int i = 0; i < K; i++) v = v*10 + (s[picks[i]]-'0');
        return v;
    }
    long long best = 0;
    for (int i = start; i <= n - (K - depth); i++) {
        picks[depth] = i;
        long long v = best_brute_k(s, n, K, picks, depth+1, i+1);
        if (v > best) best = v;
    }
    return best;
}

int main(void) {
    // Test random strings
    srand(42);
    for (int test = 0; test < 100000; test++) {
        int n = 2 + rand() % 14; // length 2..15
        char s[16];
        for (int i = 0; i < n; i++) s[i] = '1' + rand() % 9;
        s[n] = '\0';
        
        // Part 1: K=2
        long long g = best_greedy(s, n, 2);
        long long b = best_brute2(s, n);
        if (g != b) {
            printf("MISMATCH K=2: '%s' greedy=%lld brute=%lld\n", s, g, b);
        }
        
        // Part 2: K=min(12,n)
        if (n >= 12) {
            int picks[12];
            long long g2 = best_greedy(s, n, 12);
            long long b2 = best_brute_k(s, n, 12, picks, 0, 0);
            if (g2 != b2) {
                printf("MISMATCH K=12: '%s' greedy=%lld brute=%lld\n", s, g2, b2);
            }
        }
    }
    printf("Done testing\n");
    return 0;
}
