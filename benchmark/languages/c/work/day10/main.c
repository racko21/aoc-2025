#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* =========================================================
   PARSING
   ========================================================= */

#define MAX_LIGHTS  12
#define MAX_BUTTONS 16
#define MAX_JOLT    12

typedef struct {
    int nlights;
    int target_light;
    int nbuttons;
    int button[MAX_BUTTONS];
    int njolts;
    long long jolt_target[MAX_JOLT];
} Machine;

static int parse_line(const char *line, Machine *m) {
    memset(m, 0, sizeof(*m));
    const char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '[') return 0;
    p++;
    m->nlights = 0;
    m->target_light = 0;
    while (*p && *p != ']') {
        if (*p == '#') m->target_light |= (1 << m->nlights);
        m->nlights++;
        p++;
    }
    if (*p == ']') p++;
    m->nbuttons = 0;
    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '(') {
            p++;
            int mask = 0;
            while (*p && *p != ')') {
                if (*p >= '0' && *p <= '9') {
                    int idx = 0;
                    while (*p >= '0' && *p <= '9') { idx = idx*10 + (*p-'0'); p++; }
                    mask |= (1 << idx);
                } else p++;
            }
            if (*p == ')') p++;
            m->button[m->nbuttons++] = mask;
        } else if (*p == '{') {
            p++;
            m->njolts = 0;
            while (*p && *p != '}') {
                if (*p >= '0' && *p <= '9') {
                    long long val = 0;
                    while (*p >= '0' && *p <= '9') { val = val*10 + (*p-'0'); p++; }
                    m->jolt_target[m->njolts++] = val;
                } else p++;
            }
            if (*p == '}') p++;
        } else if (*p == '\0' || *p == '\n' || *p == '\r') break;
        else p++;
    }
    return 1;
}

/* =========================================================
   PART 1: GF(2), minimize weight, enumerate all 2^n subsets
   ========================================================= */

static long long solve_part1(Machine *m) {
    int n = m->nbuttons;
    int target = m->target_light;
    int best = n + 1;
    for (int mask = 0; mask < (1 << n); mask++) {
        int cnt = __builtin_popcount(mask);
        if (cnt >= best) continue;
        int xv = 0;
        for (int j = 0; j < n; j++)
            if (mask & (1 << j)) xv ^= m->button[j];
        if (xv == target) best = cnt;
    }
    return (best == n+1) ? 0LL : (long long)best;
}

/* =========================================================
   PART 2: Two-phase simplex (minimization)
   
   Minimize sum(x_j) s.t. A*x = b, x >= 0
   
   Convention:
   - tab[i][j] for i=0..m-1: constraint rows
   - tab[m][j]: objective row
   - tab[m][j] = reduced cost of var j (enter if < 0 for minimization)
   - tab[m][rhs] = -z (negated objective value)
   
   ========================================================= */

/* Use static arrays large enough:
   m <= MAX_JOLT = 12
   n <= MAX_BUTTONS = 16
   artificials = m <= 12
   total_cols = n + m + 1 <= 29
   obj row = row m (index 12)
   So we need tab[(m+1)][...] = tab[13][...]
*/
#define SP_ROWS  14   /* m+1, m<=12 so we need index 0..12 */
#define SP_COLS  32   /* n+m+1 <= 16+12+1=29 */
#define EPS 1e-9
#define BIG_M 1e12

/* We use a flat 2D array to avoid the compiler warning */
static double gtab[SP_ROWS][SP_COLS];
static int gbasis[SP_ROWS];

static void spivot(int m, int ncols, int r, int c) {
    double piv = gtab[r][c];
    for (int j = 0; j < ncols; j++) gtab[r][j] /= piv;
    for (int i = 0; i <= m; i++) {
        if (i != r) {
            double f = gtab[i][c];
            if (f != 0.0) {
                for (int j = 0; j < ncols; j++)
                    gtab[i][j] -= f * gtab[r][j];
            }
        }
    }
    gbasis[r] = c;
}

static int run_simplex(int m, int ncols) {
    int rhs = ncols - 1;
    for (int iter = 0; iter < 500000; iter++) {
        int enter = -1;
        double best_rc = -EPS;
        for (int j = 0; j < rhs; j++) {
            if (gtab[m][j] < best_rc) {
                best_rc = gtab[m][j];
                enter = j;
            }
        }
        if (enter < 0) return 1; /* optimal */
        int leave = -1;
        double min_ratio = 1e30;
        for (int i = 0; i < m; i++) {
            double aij = gtab[i][enter];
            if (aij > EPS) {
                double ratio = gtab[i][rhs] / aij;
                if (ratio < min_ratio - EPS) {
                    min_ratio = ratio;
                    leave = i;
                }
            }
        }
        if (leave < 0) return 0; /* unbounded */
        spivot(m, ncols, leave, enter);
    }
    return 0;
}

static long long solve_part2(Machine *m) {
    int n = m->nbuttons;
    int nm = m->njolts;
    if (nm == 0 || n == 0) return 0LL;
    
    double A[MAX_JOLT][MAX_BUTTONS];
    double b[MAX_JOLT];
    for (int i = 0; i < nm; i++) {
        b[i] = (double)m->jolt_target[i];
        for (int j = 0; j < n; j++)
            A[i][j] = (double)((m->button[j] >> i) & 1);
    }
    
    int ncols = n + nm + 1;
    int rhs = ncols - 1;
    
    /* Clear global tableau */
    memset(gtab, 0, sizeof(gtab));
    memset(gbasis, 0, sizeof(gbasis));
    
    /* Setup constraints with artificial variables */
    for (int i = 0; i < nm; i++) {
        for (int j = 0; j < n; j++) gtab[i][j] = A[i][j];
        gtab[i][n+i] = 1.0;
        gtab[i][rhs] = b[i];
        gbasis[i] = n+i;
    }
    
    /* Phase 1 objective: minimize sum(artificials)
     * Initial reduced costs (basis = artificials, c_B = 1):
     *   x_j: 0 - sum_i A[i][j] = -sum_i A[i][j]
     *   a_k: 0 (basic)
     * tab[nm][rhs] = -z = -sum(b)
     */
    for (int j = 0; j < n; j++) {
        double rc = 0;
        for (int i = 0; i < nm; i++) rc += A[i][j];
        gtab[nm][j] = -rc;
    }
    double z0 = 0;
    for (int i = 0; i < nm; i++) z0 += b[i];
    gtab[nm][rhs] = -z0;
    
    run_simplex(nm, ncols);
    
    double ph1_z = -gtab[nm][rhs];
    if (ph1_z > 1e-6) {
        fprintf(stderr, "Infeasible (ph1_z=%f)\n", ph1_z);
        return -1LL;
    }
    
    /* Remove degenerate artificials from basis */
    for (int i = 0; i < nm; i++) {
        if (gbasis[i] >= n) {
            for (int j = 0; j < n; j++) {
                if (fabs(gtab[i][j]) > EPS) {
                    spivot(nm, ncols, i, j);
                    break;
                }
            }
        }
    }
    
    /* Phase 2: minimize sum(x_j)
     * c_j = 1 for x_j (j<n), c_j = BIG_M for artificials
     * Recompute reduced costs based on current basis.
     */
    for (int j = 0; j <= rhs; j++) gtab[nm][j] = 0.0;
    
    for (int j = 0; j < rhs; j++) {
        double cj = (j < n) ? 1.0 : BIG_M;
        double rc = cj;
        for (int i = 0; i < nm; i++) {
            double cb_i = (gbasis[i] < n) ? 1.0 : BIG_M;
            rc -= cb_i * gtab[i][j];
        }
        gtab[nm][j] = rc;
    }
    double z_cur = 0;
    for (int i = 0; i < nm; i++) {
        double cb_i = (gbasis[i] < n) ? 1.0 : BIG_M;
        z_cur += cb_i * gtab[i][rhs];
    }
    gtab[nm][rhs] = -z_cur;
    
    run_simplex(nm, ncols);
    
    double opt = -gtab[nm][rhs];
    return llround(opt);
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc >= 2) part = atoi(argv[1]);

    FILE *f = fopen("input.txt", "r");
    if (!f) { fprintf(stderr, "Cannot open input.txt\n"); return 1; }

    char line[4096];
    long long total1 = 0, total2 = 0;
    
    while (fgets(line, sizeof(line), f)) {
        int blank = 1;
        for (int i = 0; line[i]; i++) {
            if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\r') {
                blank = 0; break;
            }
        }
        if (blank) continue;
        Machine m;
        if (!parse_line(line, &m)) continue;
        if (part == 0 || part == 1) total1 += solve_part1(&m);
        if (part == 0 || part == 2) total2 += solve_part2(&m);
    }
    fclose(f);

    if (part == 0 || part == 1) printf("Part 1: %lld\n", total1);
    if (part == 0 || part == 2) printf("Part 2: %lld\n", total2);
    return 0;
}
