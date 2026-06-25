#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Greedy algorithm to pick K digits from a string of length n (in order)
 * to maximize the resulting K-digit number.
 *
 * For position k (0-indexed, picking K total digits):
 *   - We must pick one digit from indices [start .. n - (K - k)]
 *     (leaving enough digits for the remaining K-k-1 picks after)
 *   - Pick the maximum digit in that range.
 *   - Pick the FIRST occurrence of max (leftmost) to leave max room for later.
 *
 * Returns the numeric value as long long.
 */
static long long best_k_digits(const char *s, int n, int K) {
    long long val = 0;
    int start = 0;
    for (int k = 0; k < K; k++) {
        int remaining = K - k - 1;
        int end = n - 1 - remaining;
        /* find max digit in s[start..end] */
        char best = '0';
        for (int i = start; i <= end; i++) {
            if (s[i] > best) best = s[i];
        }
        /* find first occurrence of best in [start..end] */
        int chosen = start;
        for (int i = start; i <= end; i++) {
            if (s[i] == best) { chosen = i; break; }
        }
        val = val * 10 + (s[chosen] - '0');
        start = chosen + 1;
    }
    return val;
}

/* Read a line dynamically - returns length, 0 on blank, -1 on EOF */
static int read_line(FILE *f, char **buf, size_t *cap) {
    size_t len = 0;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') break;
        if (c == '\r') continue; /* skip CR in CRLF */
        /* ensure capacity */
        if (len + 2 > *cap) {
            size_t newcap = (*cap == 0) ? 1024 : (*cap * 2);
            char *newbuf = realloc(*buf, newcap);
            if (!newbuf) { fprintf(stderr, "OOM\n"); exit(1); }
            *buf = newbuf;
            *cap = newcap;
        }
        (*buf)[len++] = (char)c;
    }
    if (len == 0 && c == EOF) return -1;
    if (*cap == 0) {
        *buf = malloc(1);
        if (!*buf) { fprintf(stderr, "OOM\n"); exit(1); }
        *cap = 1;
    }
    (*buf)[len] = '\0';
    return (int)len;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc >= 2) part = atoi(argv[1]);

    FILE *f = fopen("input.txt", "r");
    if (!f) { fprintf(stderr, "Cannot open input.txt\n"); return 1; }

    long long total1 = 0, total2 = 0;

    char *line = NULL;
    size_t cap = 0;
    int len;

    while ((len = read_line(f, &line, &cap)) != -1) {
        if (len == 0) continue; /* blank line */

        /* Only count digit characters */
        /* Count how many digits we have */
        int n = 0;
        for (int i = 0; i < len; i++) {
            if (line[i] >= '1' && line[i] <= '9') n++;
        }
        
        /* Build a clean digit-only string if needed */
        /* For now assume all chars are digits (1-9) */
        /* Use len as n */
        n = len;

        /* Part 1: pick 2 digits */
        if (n >= 2) {
            total1 += best_k_digits(line, n, 2);
        }

        /* Part 2: pick 12 digits */
        if (n >= 12) {
            total2 += best_k_digits(line, n, 12);
        }
    }

    free(line);
    fclose(f);

    if (part == 1 || part == 0) printf("Part 1: %lld\n", total1);
    if (part == 2 || part == 0) printf("Part 2: %lld\n", total2);

    return 0;
}
