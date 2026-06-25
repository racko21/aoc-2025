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
    FILE *f = fopen("example.txt","r");
    char line[512];
    
    RawShape raw_shapes[MAX_SHAPES];
    int num_raw = 0;
    int in_shape = 0;
    int cur_row = 0;
    
    while (fgets(line, sizeof(line), f)) {
        int len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = 0;
        
        printf("LINE: [%s] len=%d\n", line, len);
        
        if (len == 0) {
            if (in_shape) in_shape = 0;
            continue;
        }
        
        if (line[0] == '#' || line[0] == '.') {
            if (in_shape && num_raw > 0) {
                RawShape *rs = &raw_shapes[num_raw - 1];
                for (int col = 0; col < len; col++) {
                    if (line[col] == '#') {
                        rs->r[rs->n] = cur_row;
                        rs->c[rs->n] = col;
                        rs->n++;
                    }
                }
            }
            cur_row++;
            continue;
        }
        
        // Check for shape header
        int pos = 0;
        while (pos < len && line[pos] >= '0' && line[pos] <= '9') pos++;
        if (pos > 0 && pos < len && line[pos] == ':') {
            if (num_raw < MAX_SHAPES) {
                raw_shapes[num_raw].n = 0;
                num_raw++;
            }
            in_shape = 1;
            cur_row = 0;
            printf("  -> shape header, now %d shapes\n", num_raw);
            continue;
        }
        
        // Region line?
        if (strchr(line, 'x') && strchr(line, ':')) {
            in_shape = 0;
            printf("  -> region line\n");
            break;
        }
    }
    
    printf("\nParsed %d shapes:\n", num_raw);
    for (int i = 0; i < num_raw; i++) {
        normalize_raw(&raw_shapes[i]);
        printf("Shape %d (%d cells):", i, raw_shapes[i].n);
        for (int j = 0; j < raw_shapes[i].n; j++)
            printf(" (%d,%d)", raw_shapes[i].r[j], raw_shapes[i].c[j]);
        printf("\n");
    }
    
    fclose(f);
    return 0;
}
