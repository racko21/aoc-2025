#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROWS  14
#define MAX_COLS  30

typedef struct {
    double tab[MAX_ROWS][MAX_COLS];
    int basis[MAX_ROWS];
    int nrows;
    int ncols;
} Simplex;

static void pivot(Simplex *s, int r, int c) {
    int nrows = s->nrows;
    int ncols = s->ncols;
    double piv = s->tab[r][c];
    for (int j = 0; j < ncols; j++) s->tab[r][j] /= piv;
    for (int i = 0; i < nrows; i++) {
        if (i != r && s->tab[i][c] != 0.0) {
            double factor = s->tab[i][c];
            for (int j = 0; j < ncols; j++) s->tab[i][j] -= factor * s->tab[r][j];
        }
    }
    s->basis[r] = c;
}

static int simplex_iterate(Simplex *s) {
    int maxiter = 100000;
    int nrows = s->nrows;
    int ncols = s->ncols;
    int rhs_col = ncols - 1;
    for (int iter = 0; iter < maxiter; iter++) {
        int enter = -1;
        double min_rc = -1e-9;
        for (int j = 0; j < rhs_col; j++) {
            if (s->tab[nrows][j] < min_rc) {
                min_rc = s->tab[nrows][j];
                enter = j;
            }
        }
        if (enter == -1) return 1;
        int leave = -1;
        double min_ratio = 1e18;
        for (int i = 0; i < nrows; i++) {
            if (s->tab[i][enter] > 1e-9) {
                double ratio = s->tab[i][rhs_col] / s->tab[i][enter];
                if (ratio < min_ratio - 1e-12) {
                    min_ratio = ratio;
                    leave = i;
                }
            }
        }
        if (leave == -1) return 0;
        pivot(s, leave, enter);
    }
    return 0;
}

int main() {
    /* Machine 1: {3,5,4,7}
       Buttons: (3) (1,3) (2) (2,3) (0,2) (0,1)
       Counters: 0,1,2,3
       
       A[counter][button]:
       button 0 = (3): affects counter 3
       button 1 = (1,3): affects counters 1,3
       button 2 = (2): affects counter 2
       button 3 = (2,3): affects counters 2,3
       button 4 = (0,2): affects counters 0,2
       button 5 = (0,1): affects counters 0,1
       
       A = 
         b0 b1 b2 b3 b4 b5
       c0: 0  0  0  0  1  1  = 3
       c1: 0  1  0  0  0  1  = 5
       c2: 0  0  1  1  1  0  = 4
       c3: 1  1  0  1  0  0  = 7
    */
    
    int m = 4, n = 6;
    double A[4][6] = {
        {0, 0, 0, 0, 1, 1},  /* counter 0 */
        {0, 1, 0, 0, 0, 1},  /* counter 1 */
        {0, 0, 1, 1, 1, 0},  /* counter 2 */
        {1, 1, 0, 1, 0, 0},  /* counter 3 */
    };
    double b[4] = {3, 5, 4, 7};
    
    /* Answer should be 10: b0=1, b1=3, b2=3, b3=0, b4=1, b5=2 => sum=10 */
    /* Check: c0: b4+b5=1+2=3 OK; c1: b1+b5=3+2=5 OK; c2: b2+b3+b4=3+0+1=4 OK; c3: b0+b1+b3=1+3+0=4? NO */
    /* Problem example says: pressing (3) once, (1,3) three times, (2,3) three times, (0,2) once, (0,1) twice */
    /* That's b0=1, b1=3, b2=0, b3=3, b4=1, b5=2 => sum=10 */
    /* Check: c0: b4+b5=1+2=3 OK; c1: b1+b5=3+2=5 OK; c2: b2+b3+b4=0+3+1=4 OK; c3: b0+b1+b3=1+3+3=7 OK */
    
    int ncols = n + m + 1;
    int rhs_col = ncols - 1;
    
    Simplex s;
    memset(&s, 0, sizeof(s));
    s.nrows = m;
    s.ncols = ncols;
    
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) s.tab[i][j] = A[i][j];
        s.tab[i][n + i] = 1.0;
        s.tab[i][rhs_col] = b[i];
        s.basis[i] = n + i;
    }
    
    /* Phase 1 objective: min sum(a_i) */
    /* reduced_cost[x_j] = 0 - sum_i A[i][j] (since c_B=[1..1], B=I) */
    for (int j = 0; j < n; j++) {
        double rc = 0.0;
        for (int i = 0; i < m; i++) rc -= A[i][j];
        s.tab[m][j] = rc;
    }
    s.tab[m][rhs_col] = 0.0; /* track obj separately */
    
    simplex_iterate(&s);
    
    printf("After Phase 1:\n");
    printf("Basis: ");
    for (int i = 0; i < m; i++) printf("%d ", s.basis[i]);
    printf("\n");
    printf("RHS values: ");
    for (int i = 0; i < m; i++) printf("%.2f ", s.tab[i][rhs_col]);
    printf("\n");
    
    double ph1_obj = 0.0;
    for (int i = 0; i < m; i++) {
        if (s.basis[i] >= n) ph1_obj += s.tab[i][rhs_col];
    }
    printf("Phase 1 obj (sum of artificials in basis): %.4f\n", ph1_obj);
    
    /* Phase 2 */
    for (int j = 0; j < ncols; j++) s.tab[m][j] = 0.0;
    
    /* Recompute reduced costs for Phase 2 */
    /* c[j]=1 for j<n, c[j]=0 for artificials */
    /* After Phase 1, the tableau reflects B^{-1}. */
    /* Reduced cost for non-basic j: c[j] - c_B * (column j in current tableau) */
    /* c_B[i] = (s.basis[i] < n) ? 1.0 : 0.0 */
    for (int j = 0; j < rhs_col; j++) {
        double rc = (j < n) ? 1.0 : 0.0;
        for (int i = 0; i < m; i++) {
            if (s.basis[i] < n) rc -= s.tab[i][j];
        }
        s.tab[m][j] = rc;
    }
    /* Phase 2 obj initial = sum of x_j in basis */
    double ph2_obj = 0.0;
    for (int i = 0; i < m; i++) {
        if (s.basis[i] < n) ph2_obj += s.tab[i][rhs_col];
    }
    s.tab[m][rhs_col] = ph2_obj;
    
    printf("\nPhase 2 initial obj: %.4f\n", ph2_obj);
    printf("Phase 2 obj row: ");
    for (int j = 0; j < ncols; j++) printf("%.2f ", s.tab[m][j]);
    printf("\n");
    
    simplex_iterate(&s);
    
    printf("\nAfter Phase 2:\n");
    printf("Basis: ");
    for (int i = 0; i < m; i++) printf("%d ", s.basis[i]);
    printf("\n");
    
    double obj = 0.0;
    for (int i = 0; i < m; i++) {
        if (s.basis[i] < n) obj += s.tab[i][rhs_col];
    }
    printf("Obj = %.4f (expected 10)\n", obj);
    
    return 0;
}
