#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define SMAX_M  13
#define SMAX_N  16
#define SMAX_COLS 32
#define EPS 1e-9

typedef struct {
    double tab[SMAX_M+1][SMAX_COLS];
    int basis[SMAX_M];
    int m, n, nart, ncols;
} Simp;

static void spivot(Simp *s, int r, int c) {
    double piv = s->tab[r][c];
    for (int j = 0; j < s->ncols; j++) s->tab[r][j] /= piv;
    for (int i = 0; i <= s->m; i++) {
        if (i != r) {
            double f = s->tab[i][c];
            if (fabs(f) > EPS) {
                for (int j = 0; j < s->ncols; j++)
                    s->tab[i][j] -= f * s->tab[r][j];
            }
        }
    }
    s->basis[r] = c;
}

static int run_simplex(Simp *s) {
    int rhs = s->ncols - 1;
    for (int iter = 0; iter < 500000; iter++) {
        int enter = -1;
        double best_rc = -EPS;
        for (int j = 0; j < rhs; j++) {
            if (s->tab[s->m][j] < best_rc) {
                best_rc = s->tab[s->m][j];
                enter = j;
            }
        }
        if (enter < 0) return 1;
        int leave = -1;
        double min_ratio = 1e30;
        for (int i = 0; i < s->m; i++) {
            double aij = s->tab[i][enter];
            if (aij > EPS) {
                double ratio = s->tab[i][rhs] / aij;
                if (ratio < min_ratio - EPS) {
                    min_ratio = ratio;
                    leave = i;
                }
            }
        }
        if (leave < 0) return 0;
        spivot(s, leave, enter);
    }
    return 0;
}

int main() {
    /* Machine 1: buttons (3),(1,3),(2),(2,3),(0,2),(0,1), targets {3,5,4,7} */
    /* Counters 0,1,2,3 */
    /* button 0=(3): bit3 -> affects counter 3 */
    /* button 1=(1,3): bits 1,3 -> affects counters 1,3 */
    /* button 2=(2): bit2 -> affects counter 2 */
    /* button 3=(2,3): bits 2,3 -> affects counters 2,3 */
    /* button 4=(0,2): bits 0,2 -> affects counters 0,2 */
    /* button 5=(0,1): bits 0,1 -> affects counters 0,1 */
    int n = 6, nm = 4;
    double A[4][6] = {
        {0, 0, 0, 0, 1, 1}, /* counter 0 = 3 */
        {0, 1, 0, 0, 0, 1}, /* counter 1 = 5 */
        {0, 0, 1, 1, 1, 0}, /* counter 2 = 4 */
        {1, 1, 0, 1, 0, 0}, /* counter 3 = 7 */
    };
    double b[4] = {3, 5, 4, 7};
    
    Simp s;
    memset(&s, 0, sizeof(s));
    s.m = nm; s.n = n; s.nart = nm;
    s.ncols = n + nm + 1;
    int rhs = s.ncols - 1;
    
    for (int i = 0; i < nm; i++) {
        for (int j = 0; j < n; j++) s.tab[i][j] = A[i][j];
        s.tab[i][n+i] = 1.0;
        s.tab[i][rhs] = b[i];
        s.basis[i] = n+i;
    }
    
    /* Phase 1 obj row */
    for (int j = 0; j < n; j++) {
        double rc = 0;
        for (int i = 0; i < nm; i++) rc += A[i][j];
        s.tab[nm][j] = -rc;
    }
    double z0 = 0; for (int i = 0; i < nm; i++) z0 += b[i];
    s.tab[nm][rhs] = -z0;
    
    printf("Phase 1 initial: z=%.2f, obj_row rhs=%.2f\n", z0, s.tab[nm][rhs]);
    printf("Phase 1 obj row: ");
    for (int j = 0; j <= rhs; j++) printf("%.2f ", s.tab[nm][j]);
    printf("\n");
    
    run_simplex(&s);
    
    double ph1_z = -s.tab[nm][rhs];
    printf("After Phase 1: ph1_z=%.4f (should be ~0)\n", ph1_z);
    printf("Basis: "); for (int i = 0; i < nm; i++) printf("%d ", s.basis[i]); printf("\n");
    printf("RHS: "); for (int i = 0; i < nm; i++) printf("%.2f ", s.tab[i][rhs]); printf("\n");
    
    /* Try to remove degenerate artificials */
    for (int i = 0; i < nm; i++) {
        if (s.basis[i] >= n) {
            for (int j = 0; j < n; j++) {
                if (fabs(s.tab[i][j]) > EPS) {
                    spivot(&s, i, j);
                    break;
                }
            }
        }
    }
    printf("After degenerate fix:\n");
    printf("Basis: "); for (int i = 0; i < nm; i++) printf("%d ", s.basis[i]); printf("\n");
    
    /* Phase 2 */
    for (int j = 0; j <= rhs; j++) s.tab[nm][j] = 0.0;
    for (int j = 0; j < rhs; j++) {
        double cj = (j < n) ? 1.0 : 0.0;
        double rc = cj;
        for (int i = 0; i < nm; i++) {
            double cb_i = (s.basis[i] < n) ? 1.0 : 0.0;
            rc -= cb_i * s.tab[i][j];
        }
        s.tab[nm][j] = rc;
    }
    double z_cur = 0;
    for (int i = 0; i < nm; i++) if (s.basis[i] < n) z_cur += s.tab[i][rhs];
    s.tab[nm][rhs] = -z_cur;
    
    printf("\nPhase 2 initial: z=%.2f, obj_row rhs=%.2f\n", z_cur, s.tab[nm][rhs]);
    printf("Phase 2 obj row: ");
    for (int j = 0; j <= rhs; j++) printf("%.2f ", s.tab[nm][j]);
    printf("\n");
    
    run_simplex(&s);
    
    double opt = -s.tab[nm][rhs];
    printf("Optimal: %.4f (expected 10)\n", opt);
    printf("Basis: "); for (int i = 0; i < nm; i++) printf("%d ", s.basis[i]); printf("\n");
    printf("Values: "); for (int i = 0; i < nm; i++) printf("x[%d]=%.2f ", s.basis[i], s.tab[i][rhs]); printf("\n");
    
    return 0;
}
