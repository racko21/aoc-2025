#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Try different ray casting: cast ray to the LEFT (decreasing x) from point (px, py)
// Count vertical edges that the ray crosses.
// A vertical edge from (ex, y1) to (ex, y2) is crossed if:
//   ex < px AND min(y1,y2) <= py <= max(y1,y2)
// But we need to handle the endpoints carefully.
// Standard convention: count edge if ymin <= py < ymax (include bottom, exclude top)
// This avoids double-counting at vertices.

int is_inside_polygon(int px, int py, int *xs, int *ys, int n) {
    // Check boundary first
    for (int i = 0; i < n; i++) {
        int x1 = xs[i], y1 = ys[i];
        int x2 = xs[(i+1)%n], y2 = ys[(i+1)%n];
        if (y1 == y2) {
            // Horizontal edge
            int xmin = x1 < x2 ? x1 : x2;
            int xmax = x1 > x2 ? x1 : x2;
            if (py == y1 && px >= xmin && px <= xmax) return 1;
        } else {
            // Vertical edge
            int ymin = y1 < y2 ? y1 : y2;
            int ymax = y1 > y2 ? y1 : y2;
            if (px == x1 && py >= ymin && py <= ymax) return 1;
        }
    }
    
    // Ray casting: ray to the left
    int crossings = 0;
    for (int i = 0; i < n; i++) {
        int x1 = xs[i], y1 = ys[i];
        int x2 = xs[(i+1)%n], y2 = ys[(i+1)%n];
        if (y1 != y2) {
            // Vertical edge at x = x1
            int ymin = y1 < y2 ? y1 : y2;
            int ymax = y1 > y2 ? y1 : y2;
            // Use half-open interval: ymin <= py < ymax
            if (py >= ymin && py < ymax && x1 <= px) {
                crossings++;
            }
        }
        // Horizontal edges don't contribute to left-ray crossing
    }
    return crossings % 2 == 1;
}

int main(void) {
    int n = 8;
    int xs[] = {7, 11, 11, 9, 9, 2, 2, 7};
    int ys[] = {1, 1, 7, 7, 5, 5, 3, 3};
    
    int MAXX = 14, MAXY = 9;
    printf("Real grid (row=y, col=x):\n");
    for (int py = 0; py < MAXY; py++) {
        for (int px = 0; px < MAXX; px++) {
            int v = is_inside_polygon(px, py, xs, ys, n);
            // Check if it's a red tile
            int red = 0;
            for (int i = 0; i < n; i++) if (xs[i] == px && ys[i] == py) { red = 1; break; }
            char c = '.';
            if (red) c = '#';
            else if (v) c = 'X';
            printf("%c", c);
        }
        printf("\n");
    }
    
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
    
    // Check rectangle (2,3)-(9,5): x=[2,9], y=[3,5]
    printf("\nChecking rectangle (2,3)-(9,5): x=[2,9], y=[3,5]\n");
    int all_valid = 1;
    for (int py = 3; py <= 5; py++) {
        for (int px = 2; px <= 9; px++) {
            int v = is_inside_polygon(px, py, xs, ys, n);
            if (!v) { printf("INVALID at (%d,%d)\n", px, py); all_valid = 0; }
        }
    }
    if (all_valid) printf("All cells valid! Area = %d\n", 8*3);
    
    return 0;
}
