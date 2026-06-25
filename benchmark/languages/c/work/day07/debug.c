#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXROWS 300
#define MAXCOLS 300

char grid[MAXROWS][MAXCOLS+1];
int rows, cols;

typedef struct {
    int row, col;
} Beam;

Beam queue[MAXROWS * MAXCOLS * 4];
int qhead, qtail;

int visited[MAXROWS][MAXCOLS];
long long part1_splits;

int main() {
    FILE *f = fopen("input.txt", "r");
    rows = 0;
    while (fgets(grid[rows], MAXCOLS+1, f)) {
        int len = strlen(grid[rows]);
        while (len > 0 && (grid[rows][len-1] == '\n' || grid[rows][len-1] == '\r'))
            grid[rows][--len] = '\0';
        if (len > 0) { cols = len; rows++; }
    }
    fclose(f);
    
    int s_row = -1, s_col = -1;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            if (grid[r][c] == 'S') { s_row = r; s_col = c; }
    
    memset(visited, 0, sizeof(visited));
    part1_splits = 0;
    qhead = qtail = 0;
    queue[qtail++] = (Beam){s_row, s_col};
    visited[s_row][s_col] = 1;
    
    while (qhead < qtail) {
        Beam b = queue[qhead++];
        int r = b.row, c = b.col;
        
        int hit = -1;
        for (int nr = r; nr < rows; nr++) {
            if (grid[nr][c] == '^') { hit = nr; break; }
        }
        
        if (hit >= 0) {
            part1_splits++;
            printf("Beam at (%d,%d) hits splitter at (%d,%d)\n", r, c, hit, c);
            
            int lc = c - 1, rc2 = c + 1;
            if (lc >= 0 && !visited[hit][lc]) {
                visited[hit][lc] = 1;
                queue[qtail++] = (Beam){hit, lc};
                printf("  Spawn left at (%d,%d)\n", hit, lc);
            }
            if (rc2 < cols && !visited[hit][rc2]) {
                visited[hit][rc2] = 1;
                queue[qtail++] = (Beam){hit, rc2};
                printf("  Spawn right at (%d,%d)\n", hit, rc2);
            }
        } else {
            printf("Beam at (%d,%d) exits grid\n", r, c);
        }
    }
    
    printf("Total splits: %lld\n", part1_splits);
    return 0;
}
