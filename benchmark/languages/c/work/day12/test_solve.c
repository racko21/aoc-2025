#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Simplified test: can we solve 4x4 with 2 copies of shape 4?
// Shape 4: ### / #.. / ###
// Cells: (0,0),(0,1),(0,2),(1,0),(2,0),(2,1),(2,2)

#define MAX_CELLS 10
#define MAX_W 10
#define MAX_H 10

typedef struct {
    int dr[MAX_CELLS], dc[MAX_CELLS], n, maxR, maxC;
} Orient;

Orient orients[8];
int num_orients = 0;

static char grid[MAX_H][MAX_W];
static int gW = 4, gH = 4;
static int remain[2];
static int total_rem;
static int n_types = 1;

static int find_first_empty(int *r, int *c) {
    for (int i = 0; i < gH; i++)
        for (int j = 0; j < gW; j++)
            if (!grid[i][j]) { *r=i; *c=j; return 1; }
    return 0;
}

static int place(int oi, int pr, int pc) {
    Orient *o = &orients[oi];
    for (int k = 0; k < o->n; k++) {
        int r = pr+o->dr[k], c = pc+o->dc[k];
        if (r<0||r>=gH||c<0||c>=gW||grid[r][c]) return 0;
    }
    for (int k = 0; k < o->n; k++) grid[pr+o->dr[k]][pc+o->dc[k]] = 1;
    return 1;
}
static void unplace(int oi, int pr, int pc) {
    Orient *o = &orients[oi];
    for (int k = 0; k < o->n; k++) grid[pr+o->dr[k]][pc+o->dc[k]] = 0;
}

int solve() {
    int fr, fc;
    if (!find_first_empty(&fr, &fc)) return (total_rem == 0);
    if (total_rem == 0) return 1;
    
    for (int oi = 0; oi < num_orients; oi++) {
        Orient *o = &orients[oi];
        for (int k = 0; k < o->n; k++) {
            int pr = fr - o->dr[k];
            int pc = fc - o->dc[k];
            if (pr<0||pr+o->maxR>=gH||pc<0||pc+o->maxC>=gW) continue;
            
            int valid = 1;
            int fr_idx = fr*gW+fc;
            for (int j = 0; j < o->n; j++) {
                if (j == k) continue;
                int idx = (pr+o->dr[j])*gW + (pc+o->dc[j]);
                if (idx < fr_idx) { valid=0; break; }
            }
            if (!valid) continue;
            
            if (!place(oi, pr, pc)) continue;
            remain[0]--;
            total_rem--;
            if (remain[0] >= 0) {
                if (solve()) {
                    unplace(oi, pr, pc);
                    remain[0]++;
                    total_rem++;
                    return 1;
                }
            }
            unplace(oi, pr, pc);
            remain[0]++;
            total_rem++;
        }
    }
    return 0;
}

int main() {
    // Shape 4 orientations:
    // Orientation 0: (0,0)(0,1)(0,2)(1,0)(2,0)(2,1)(2,2)
    // Orientation 1 (flipped): same due to symmetry? Actually shape 4 from example
    // is ### / #.. / ### which is symmetric vertically but not horizontally
    // Let me add both orientations manually
    
    // Orig: ###/#../###
    orients[0].dr[0]=0;orients[0].dc[0]=0;
    orients[0].dr[1]=0;orients[0].dc[1]=1;
    orients[0].dr[2]=0;orients[0].dc[2]=2;
    orients[0].dr[3]=1;orients[0].dc[3]=0;
    orients[0].dr[4]=2;orients[0].dc[4]=0;
    orients[0].dr[5]=2;orients[0].dc[5]=1;
    orients[0].dr[6]=2;orients[0].dc[6]=2;
    orients[0].n=7; orients[0].maxR=2; orients[0].maxC=2;
    
    // 90° CW: transpose+flip = column becomes row
    // (r,c)->(c, maxR-r) for 3 rows: (r,c)->(c,2-r)
    // (0,0)->(0,2),(0,1)->(1,2),(0,2)->(2,2),(1,0)->(0,1),(2,0)->(0,0),(2,1)->(1,0),(2,2)->(2,0)
    // Normalize (already min at (0,0)):
    // (0,0)(0,1)(0,2)(1,0)(1,2)(2,0)(2,2)... 
    // Wait: after rotation we get: (0,2),(1,2),(2,2),(0,1),(0,0),(1,0),(2,0)
    // = (0,0)(0,1)(0,2)(1,0)(1,2)(2,0)(2,2) - that's shape 0 of the example! 
    // Actually let me just hardcode both shapes from example shape 4
    // Looking at the example: shape 4 is ### / #.. / ###
    // After 90° rotation (CW):
    // col 0: # in rows 0,1,2 = col becomes row: row 0 has cells from original col 0
    // Original col 0: rows 0,1,2 → all are # 
    // Original col 1: rows 0,2 are #, row 1 is .
    // Original col 2: rows 0,2 are #, row 1 is .
    // After 90° CW: new_grid[c][maxR-r] = old[r][c]
    // new_grid[0][2]=old[0][0]=#, new_grid[0][1]=old[1][0]=#, new_grid[0][0]=old[2][0]=#
    // new_grid[1][2]=old[0][1]=#, new_grid[1][1]=old[1][1]=., new_grid[1][0]=old[2][1]=#
    // new_grid[2][2]=old[0][2]=#, new_grid[2][1]=old[1][2]=., new_grid[2][0]=old[2][2]=#
    // new_grid = ###/### ? No: 
    // Row 0: (0,0)#,(0,1)#,(0,2)#
    // Row 1: (1,0)#,(1,1).,(1,2)#
    // Row 2: (2,0)#,(2,1).,(2,2)#
    // = ###/#.#/#.# = shape 0 from example (but not shape 4)
    // Let me just check: is 90° rotation of shape 4 the same as original shape 4?
    // No. So shape 4 has 4 unique orientations.
    
    // For this test, just use orientation 0
    num_orients = 1;
    remain[0] = 2;
    total_rem = 2;
    
    printf("Trying 4x4 with 2 copies of shape 4:\n");
    int r = solve();
    printf("Result: %s\n", r ? "feasible" : "infeasible");
    
    return 0;
}
