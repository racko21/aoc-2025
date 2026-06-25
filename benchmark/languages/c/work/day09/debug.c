#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long long ll;

static int cmp_int(const void *a, const void *b) {
    int x = *(int*)a, y = *(int*)b;
    return x < y ? -1 : x > y ? 1 : 0;
}

static int unique(int *arr, int n) {
    if (n == 0) return 0;
    int k = 0;
    for (int i = 0; i < n; i++) {
        if (i == 0 || arr[i] != arr[i-1]) arr[k++] = arr[i];
    }
    return k;
}

static int find_idx(int *arr, int n, int val) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (arr[mid] == val) return mid;
        else if (arr[mid] < val) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

int main(void) {
    // Example
    int n = 8;
    int xs[] = {7, 11, 11, 9, 9, 2, 2, 7};
    int ys[] = {1, 1, 7, 7, 5, 5, 3, 3};
    
    // Print valid grid in real coordinates (0..13 x 0..8)
    int MAXX = 14, MAXY = 9;
    printf("Real grid (row=y, col=x):\n");
    for (int py = 0; py < MAXY; py++) {
        for (int px = 0; px < MAXX; px++) {
            // Check if (px, py) is on boundary or inside
            int on_boundary = 0;
            int crossings = 0;
            for (int i = 0; i < n; i++) {
                int x1 = xs[i], y1 = ys[i];
                int x2 = xs[(i+1)%n], y2 = ys[(i+1)%n];
                if (y1 == y2) {
                    // Horizontal edge
                    int xmin = x1 < x2 ? x1 : x2;
                    int xmax = x1 > x2 ? x1 : x2;
                    if (py == y1 && px >= xmin && px <= xmax) {
                        on_boundary = 1; break;
                    }
                    if (y1 < py && px >= xmin && px <= xmax) crossings++;
                } else {
                    // Vertical edge
                    int ymin = y1 < y2 ? y1 : y2;
                    int ymax = y1 > y2 ? y1 : y2;
                    if (px == x1 && py >= ymin && py <= ymax) {
                        on_boundary = 1; break;
                    }
                }
            }
            char c = '.';
            if (on_boundary) c = '#';
            else if (crossings % 2 == 1) c = 'X';
            printf("%c", c);
        }
        printf("\n");
    }
    
    // Expected from problem:
    // ..............
    // .......#XXX#..
    // .......XXXXX..
    // ..#XXXX#XXXX..
    // ..XXXXXXXXXX..
    // ..#XXXXXX#XX..
    // .........XXX..
    // .........#X#..
    // ..............
    // (where # = red/boundary, X = green/interior)
    
    printf("\nExpected:\n");
    printf("..............\n");
    printf(".......#XXX#..\n");
    printf(".......XXXXX..\n");
    printf("..#XXXX#XXXX..\n");
    printf("..XXXXXXXXXX..\n");
    printf("..#XXXXXX#XX..\n");
    printf(".........XXX..\n");
    printf(".........#X#..\n");
    printf("..............\n");
    
    // Now check the pair (9,5) and (2,3) - expected area 24
    // Rectangle: x in [2,9], y in [3,5] -> area = 8*3 = 24
    printf("\nChecking rectangle (2,3)-(9,5): x=[2,9], y=[3,5]\n");
    printf("Expected: area = 8*3 = %d\n", 8*3);
    for (int py = 3; py <= 5; py++) {
        for (int px = 2; px <= 9; px++) {
            int on_boundary = 0;
            int crossings = 0;
            for (int i = 0; i < n; i++) {
                int x1 = xs[i], y1 = ys[i];
                int x2 = xs[(i+1)%n], y2 = ys[(i+1)%n];
                if (y1 == y2) {
                    int xmin = x1 < x2 ? x1 : x2;
                    int xmax = x1 > x2 ? x1 : x2;
                    if (py == y1 && px >= xmin && px <= xmax) { on_boundary = 1; break; }
                    if (y1 < py && px >= xmin && px <= xmax) crossings++;
                } else {
                    int ymin = y1 < y2 ? y1 : y2;
                    int ymax = y1 > y2 ? y1 : y2;
                    if (px == x1 && py >= ymin && py <= ymax) { on_boundary = 1; break; }
                }
            }
            int valid = on_boundary || (crossings % 2 == 1);
            printf("(%d,%d): %s\n", px, py, valid ? "valid" : "INVALID");
        }
    }
    
    return 0;
}
