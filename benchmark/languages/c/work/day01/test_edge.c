#include <stdio.h>
// Test: R1000 from 50 should pass through 0 exactly 10 times
// first_k = 100 - 50 = 50
// count = (1000 - 50) / 100 + 1 = 950/100 + 1 = 9 + 1 = 10
int main() {
    int pos = 50, dist = 1000;
    int first_k = (pos == 0) ? 100 : (100 - pos);
    long long count = 0;
    if (dist >= first_k) count = (long long)(dist - first_k) / 100 + 1;
    printf("R1000 from 50: passes through 0 = %lld times (expected 10)\n", count);
    // New position: (50 + 1000) % 100 = 1050 % 100 = 50
    printf("New position: %d (expected 50)\n", (50 + 1000) % 100);
    return 0;
}
