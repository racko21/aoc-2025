#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SHAPES 20
#define MAX_CELLS_PER_SHAPE 12
#define MAX_ORIENTS 8
#define MAX_W 55
#define MAX_H 55
#define MAX_TYPES 20

typedef struct {
    int dr[MAX_CELLS_PER_SHAPE];
    int dc[MAX_CELLS_PER_SHAPE];
    int n;
    int maxR;
    int maxC;
} Orient;

typedef struct {
    Orient orients[MAX_ORIENTS];
    int num_orients;
    int num_cells;
} ShapeSet;

int num_shapes = 0;
ShapeSet shapes[MAX_SHAPES];

typedef struct {
    int r[MAX_CELLS_PER_SHAPE];
    int c[MAX_CELLS_PER_SHAPE];
    int n;
} RawShape;

static void normalize_raw(RawShape *s) {
    int minr = 999, minc = 999;
    for (int i = 0; i < s->n; i++) {
        if (s->r[i] < minr) minr = s->r[i];
        if (s->c[i] < minc) minc = s->c[i];
    }
    for (int i = 0; i < s->n; i++) { s->r[i] -= minr; s->c[i] -= minc; }
    for (int i = 0; i < s->n - 1; i++)
        for (int j = i + 1; j < s->n; j++)
            if (s->r[j] < s->r[i] || (s->r[j] == s->r[i] && s->c[j] < s->c[i])) {
                int t = s->r[i]; s->r[i] = s->r[j]; s->r[j] = t;
                t = s->c[i]; s->c[i] = s->c[j]; s->c[j] = t;
            }
}

int main() {
    // Parse example.txt manually to debug
    // Shape 0: ### ##. ##.
    RawShape r0 = {{0,0,0,1,1,2,2},{0,1,2,0,1,0,1},7};
    normalize_raw(&r0);
    printf("Shape 0:");
    for(int i=0;i<r0.n;i++) printf(" (%d,%d)",r0.r[i],r0.c[i]);
    printf("\n");
    
    return 0;
}
