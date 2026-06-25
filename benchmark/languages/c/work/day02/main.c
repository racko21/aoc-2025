#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef unsigned long long ull;
typedef long long ll;

/* Check if a number (given as string) is a valid repeated pattern:
   the string must be s repeated k>=2 times, with no leading zeros.
   For part1: k must be exactly 2.
   For part2: k >= 2 (any repetition count).
*/

/* Check if string of length len is s (length period) repeated (len/period) times */
static int is_repeated(const char *buf, int len, int period) {
    if (len % period != 0) return 0;
    int reps = len / period;
    if (reps < 2) return 0;
    for (int i = period; i < len; i++) {
        if (buf[i] != buf[i % period]) return 0;
    }
    return 1;
}

/* Is this number invalid under part1 rules?
   = string of even length, first half == second half, no leading zero */
static int is_invalid_p1(ull n) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%llu", n);
    if (len % 2 != 0) return 0;
    if (buf[0] == '0') return 0; /* no leading zeros - can't happen for n>0 anyway */
    int half = len / 2;
    return (memcmp(buf, buf + half, half) == 0) ? 1 : 0;
}

/* Is this number invalid under part2 rules?
   = string of length L, exists period d | L, d < L, d >= 1, repeated L/d >= 2 times, no leading zero */
static int is_invalid_p2(ull n) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%llu", n);
    if (buf[0] == '0') return 0;
    /* Try all divisors d of len where d < len */
    for (int d = 1; d < len; d++) {
        if (len % d == 0) {
            if (is_repeated(buf, len, d)) return 1;
        }
    }
    return 0;
}

/* 
   Strategy: For each digit length L, generate all invalid IDs of that length
   and check if they fall within any range.
   
   For Part 1: L must be even. The first half x ranges from 10^(L/2-1) to 10^(L/2)-1.
               The invalid ID is x * 10^(L/2) + x.
   
   For Part 2: For each length L, for each divisor d of L (d < L), generate
               all numbers of period exactly d (i.e., the string is s repeated L/d times).
               Use a set to avoid double counting.
               
   We need to handle large ranges. The input has numbers up to ~6.8e9 (10 digits).
   So IDs can have at most 10 digits.
   
   For Part 1: max half-length = 5 (10-digit IDs).
   For Part 2: we need all repeated patterns up to 10 digits.
   
   Let's enumerate all invalid IDs up to say 10^11 (11 digits to be safe),
   collect them, sort them, then for each range do a binary search.
*/

#define MAX_INVALID 2000000

static ull invalid_p1[MAX_INVALID];
static int n_invalid_p1 = 0;

static ull invalid_p2[MAX_INVALID];
static int n_invalid_p2 = 0;

static int cmp_ull(const void *a, const void *b) {
    ull x = *(ull*)a, y = *(ull*)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

/* Add to array if not already present (we'll sort+dedup later) */
static void add_p1(ull v) {
    if (n_invalid_p1 < MAX_INVALID)
        invalid_p1[n_invalid_p1++] = v;
}

static void add_p2(ull v) {
    if (n_invalid_p2 < MAX_INVALID)
        invalid_p2[n_invalid_p2++] = v;
}

/* Power of 10 */
static ull pow10(int e) {
    ull r = 1;
    for (int i = 0; i < e; i++) r *= 10;
    return r;
}

/* Generate all invalid IDs for Part 1:
   For each half-length h from 1 to MAX_H:
     x from 10^(h-1) to 10^h - 1 (or 1 if h==1 for single digit... wait, h=1: x from 1 to 9)
     ID = x * 10^h + x
*/
static void generate_p1(ull limit) {
    for (int h = 1; h <= 10; h++) {
        ull base = pow10(h);
        ull x_lo = (h == 1) ? 1 : pow10(h-1);
        ull x_hi = pow10(h) - 1;
        for (ull x = x_lo; x <= x_hi; x++) {
            ull id = x * base + x;
            if (id > limit) break;
            add_p1(id);
        }
        /* If even the smallest ID of this h exceeds limit, stop */
        ull smallest = x_lo * base + x_lo;
        if (smallest > limit) break;
    }
}

/* Generate all invalid IDs for Part 2:
   For each total length L from 2 to MAX_L:
     For each divisor d of L, d < L (period = d):
       Generate all strings of length L with minimal period d
       (i.e., repeated d-length pattern, where the d-length pattern has
        no smaller period... actually we don't need minimal period, 
        we just need to generate all and dedup)
       
   Actually, simpler: for each period d from 1 to 9, and each rep count r >= 2:
     total length L = d * r
     x from 10^(d-1) to 10^d - 1 (for d>=2; for d=1 from 1 to 9)
     ID = x repeated r times = x * (10^(d*(r-1)) + 10^(d*(r-2)) + ... + 1)
        = x * (10^(d*r) - 1) / (10^d - 1)
     
   We generate all and dedup.
*/
static void generate_p2(ull limit) {
    /* period d from 1 to 10, repetitions r from 2 upward */
    for (int d = 1; d <= 10; d++) {
        ull x_lo = (d == 1) ? 1 : pow10(d-1);
        ull x_hi = pow10(d) - 1;
        ull base_d = pow10(d);
        
        for (int r = 2; r <= 20; r++) {
            int total_len = d * r;
            if (total_len > 20) break;
            
            /* multiplier = 10^(d*(r-1)) + ... + 10^0 */
            /* = (base_d^r - 1) / (base_d - 1) */
            /* Compute it carefully to avoid overflow */
            /* Actually let's just build it as sum */
            ull mult = 0;
            ull pw = 1;
            int overflow = 0;
            for (int i = 0; i < r; i++) {
                ull new_mult = mult + pw;
                if (new_mult < mult) { overflow = 1; break; }
                mult = new_mult;
                ull new_pw = pw * base_d;
                if (new_pw < pw && i < r-1) { overflow = 1; break; }
                pw = new_pw;
            }
            if (overflow) break;
            
            /* Check if x_lo * mult <= limit */
            /* x_lo * mult might overflow too */
            /* Use __uint128_t for overflow check */
            __uint128_t lo128 = (__uint128_t)x_lo * mult;
            if (lo128 > (__uint128_t)limit) break;
            
            for (ull x = x_lo; x <= x_hi; x++) {
                __uint128_t id128 = (__uint128_t)x * mult;
                if (id128 > (__uint128_t)limit) break;
                ull id = (ull)id128;
                add_p2(id);
            }
        }
    }
}

/* Sort and dedup an array */
static int sort_dedup(ull *arr, int n) {
    qsort(arr, n, sizeof(ull), cmp_ull);
    int j = 0;
    for (int i = 0; i < n; i++) {
        if (i == 0 || arr[i] != arr[i-1])
            arr[j++] = arr[i];
    }
    return j;
}

/* Sum all elements in sorted arr[] that are in [lo, hi] */
static ll sum_in_range(ull *arr, int n, ull lo, ull hi) {
    /* Binary search for first >= lo */
    int left = 0, right = n;
    while (left < right) {
        int mid = (left + right) / 2;
        if (arr[mid] < lo) left = mid + 1;
        else right = mid;
    }
    ll s = 0;
    for (int i = left; i < n && arr[i] <= hi; i++) {
        s += (ll)arr[i];
    }
    return s;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc >= 2) part = atoi(argv[1]);

    /* Read input */
    FILE *f = fopen("input.txt", "r");
    if (!f) { perror("input.txt"); return 1; }
    
    /* Read entire file */
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);
    char *buf = malloc(fsize + 2);
    fread(buf, 1, fsize, f);
    buf[fsize] = '\0';
    fclose(f);
    
    /* Find max value in ranges to set limit */
    ull global_max = 0;
    {
        char *p = buf;
        while (*p) {
            while (*p && !(*p >= '0' && *p <= '9')) p++;
            if (!*p) break;
            ull v = 0;
            while (*p >= '0' && *p <= '9') v = v*10 + (*p++ - '0');
            if (v > global_max) global_max = v;
        }
    }
    
    /* Generate invalid IDs */
    generate_p1(global_max);
    n_invalid_p1 = sort_dedup(invalid_p1, n_invalid_p1);
    
    generate_p2(global_max);
    n_invalid_p2 = sort_dedup(invalid_p2, n_invalid_p2);
    
    /* Parse ranges and accumulate sums */
    ll sum1 = 0, sum2 = 0;
    
    char *p = buf;
    while (*p) {
        /* Skip non-digits */
        while (*p && !(*p >= '0' && *p <= '9')) p++;
        if (!*p) break;
        ull lo = 0;
        while (*p >= '0' && *p <= '9') lo = lo*10 + (*p++ - '0');
        /* expect '-' */
        if (*p == '-') p++;
        ull hi = 0;
        while (*p >= '0' && *p <= '9') hi = hi*10 + (*p++ - '0');
        
        if (part == 0 || part == 1)
            sum1 += sum_in_range(invalid_p1, n_invalid_p1, lo, hi);
        if (part == 0 || part == 2)
            sum2 += sum_in_range(invalid_p2, n_invalid_p2, lo, hi);
    }
    
    free(buf);
    
    if (part == 0 || part == 1)
        printf("Part 1: %lld\n", sum1);
    if (part == 0 || part == 2)
        printf("Part 2: %lld\n", sum2);
    
    return 0;
}
