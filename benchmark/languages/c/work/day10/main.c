#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

/* ============================================================
   PARSING
   ============================================================ */

#define MAX_LIGHTS 16
#define MAX_BUTTONS 16
#define MAX_JOLTS 16

typedef struct {
    int n_lights;
    int target[MAX_LIGHTS];     // 0 or 1
    int n_buttons;
    int n_cols[MAX_BUTTONS];    // number of lights each button toggles
    int cols[MAX_BUTTONS][MAX_LIGHTS];
    int n_jolts;
    int jolts[MAX_JOLTS];
} Machine;

static int parse_line(const char *line, Machine *m) {
    memset(m, 0, sizeof(*m));
    const char *p = line;
    
    // Parse [indicator lights]
    while (*p && *p != '[') p++;
    if (!*p) return 0;
    p++; // skip '['
    m->n_lights = 0;
    while (*p && *p != ']') {
        if (*p == '.') m->target[m->n_lights++] = 0;
        else if (*p == '#') m->target[m->n_lights++] = 1;
        p++;
    }
    if (*p == ']') p++;
    
    // Parse buttons (parentheses)
    m->n_buttons = 0;
    while (*p) {
        while (*p && *p != '(' && *p != '{') p++;
        if (!*p || *p == '{') break;
        p++; // skip '('
        m->n_cols[m->n_buttons] = 0;
        while (*p && *p != ')') {
            if (*p >= '0' && *p <= '9') {
                int val = 0;
                while (*p >= '0' && *p <= '9') val = val*10 + (*p++ - '0');
                m->cols[m->n_buttons][m->n_cols[m->n_buttons]++] = val;
            } else {
                p++;
            }
        }
        if (*p == ')') p++;
        m->n_buttons++;
    }
    
    // Parse joltage {curly braces}
    m->n_jolts = 0;
    while (*p && *p != '{') p++;
    if (*p == '{') {
        p++;
        while (*p && *p != '}') {
            if (*p >= '0' && *p <= '9') {
                int val = 0;
                while (*p >= '0' && *p <= '9') val = val*10 + (*p++ - '0');
                m->jolts[m->n_jolts++] = val;
            } else {
                p++;
            }
        }
    }
    
    return 1;
}

/* ============================================================
   PART 1: GF(2) system, minimize total presses
   Each button pressed 0 or 1 times (mod 2 is what matters)
   But we can press any number of times -- pressing even times = 0 effect.
   So effectively x_i ∈ {0,1} for the toggle effect.
   We need: sum_j (A[i][j] * x_j) = target[i]  (mod 2) for all lights i
   Minimize sum(x_j).
   
   Algorithm:
   1. Build A (n_lights x n_buttons) over GF(2), b = target vector
   2. Gaussian elimination to find particular solution + null space basis
   3. Enumerate 2^(dim null space) combinations, find min weight
   ============================================================ */

// Gaussian elimination over GF(2)
// A is m x n matrix, b is m-vector
// Returns: -1 if no solution, else number of free variables
// Sets x_particular to one solution
// null_basis[k][0..n-1] = k-th null space vector

static int gf2_solve(int m, int n,
                     int A[MAX_LIGHTS][MAX_BUTTONS],
                     int b[MAX_LIGHTS],
                     int x_part[MAX_BUTTONS],
                     int null_basis[MAX_BUTTONS][MAX_BUTTONS],
                     int *null_dim)
{
    // Work on augmented matrix [A|b]
    int mat[MAX_LIGHTS][MAX_BUTTONS + 1];
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) mat[i][j] = A[i][j];
        mat[i][n] = b[i];
    }
    
    int pivot_col[MAX_LIGHTS]; // pivot_col[row] = which column is the pivot
    int is_pivot[MAX_BUTTONS];
    memset(pivot_col, -1, sizeof(pivot_col));
    memset(is_pivot, 0, sizeof(is_pivot));
    
    int cur_row = 0;
    int pivot_row[MAX_BUTTONS]; // pivot_row[col] = which row pivoted on col
    memset(pivot_row, -1, sizeof(pivot_row));
    
    for (int col = 0; col < n && cur_row < m; col++) {
        // Find pivot
        int pr = -1;
        for (int row = cur_row; row < m; row++) {
            if (mat[row][col]) { pr = row; break; }
        }
        if (pr < 0) continue;
        
        // Swap rows
        if (pr != cur_row) {
            for (int j = 0; j <= n; j++) {
                int tmp = mat[cur_row][j];
                mat[cur_row][j] = mat[pr][j];
                mat[pr][j] = tmp;
            }
        }
        
        // Eliminate
        for (int row = 0; row < m; row++) {
            if (row != cur_row && mat[row][col]) {
                for (int j = 0; j <= n; j++)
                    mat[row][j] ^= mat[cur_row][j];
            }
        }
        
        pivot_col[cur_row] = col;
        is_pivot[col] = 1;
        pivot_row[col] = cur_row;
        cur_row++;
    }
    
    // Check consistency
    for (int row = cur_row; row < m; row++) {
        if (mat[row][n]) return -1; // no solution
    }
    
    // Build particular solution (free vars = 0)
    memset(x_part, 0, n * sizeof(int));
    for (int row = 0; row < cur_row; row++) {
        int col = pivot_col[row];
        if (col >= 0) x_part[col] = mat[row][n];
    }
    
    // Build null space
    *null_dim = 0;
    for (int col = 0; col < n; col++) {
        if (!is_pivot[col]) {
            // Free variable: set it to 1, all others to 0
            int vec[MAX_BUTTONS];
            memset(vec, 0, n * sizeof(int));
            vec[col] = 1;
            // For each pivot row, compute the pivot variable's value
            for (int row = 0; row < cur_row; row++) {
                int pc = pivot_col[row];
                if (pc >= 0) {
                    // pivot variable = -sum of non-pivot contributions (mod 2)
                    // but we only vary 'col', others are 0
                    vec[pc] = mat[row][col];
                }
            }
            memcpy(null_basis[*null_dim], vec, n * sizeof(int));
            (*null_dim)++;
        }
    }
    
    return cur_row;
}

static long long part1_machine(Machine *m) {
    int n = m->n_buttons;
    int rows = m->n_lights;
    
    // Build matrix A[light][button]
    int A[MAX_LIGHTS][MAX_BUTTONS];
    memset(A, 0, sizeof(A));
    for (int b = 0; b < n; b++) {
        for (int k = 0; k < m->n_cols[b]; k++) {
            int light = m->cols[b][k];
            if (light < rows) A[light][b] = 1;
        }
    }
    
    int b_vec[MAX_LIGHTS];
    for (int i = 0; i < rows; i++) b_vec[i] = m->target[i];
    
    int x_part[MAX_BUTTONS];
    int null_basis[MAX_BUTTONS][MAX_BUTTONS];
    int null_dim;
    
    int rank = gf2_solve(rows, n, A, b_vec, x_part, null_basis, &null_dim);
    if (rank < 0) return -1; // no solution
    
    // Enumerate all 2^null_dim combinations
    long long best = LLONG_MAX;
    long long total = 1LL << null_dim;
    
    for (long long mask = 0; mask < total; mask++) {
        int x[MAX_BUTTONS];
        memcpy(x, x_part, n * sizeof(int));
        for (int k = 0; k < null_dim; k++) {
            if (mask & (1LL << k)) {
                for (int j = 0; j < n; j++)
                    x[j] ^= null_basis[k][j];
            }
        }
        long long sum = 0;
        for (int j = 0; j < n; j++) sum += x[j];
        if (sum < best) best = sum;
    }
    
    return best;
}

/* ============================================================
   PART 2: Integer LP
   Minimize sum(x_j) subject to Ax = t, x_j >= 0, x_j integer
   A[i][j] = 1 if button j affects counter i, else 0
   
   We use LP relaxation via simplex, then check integrality.
   If not integer, do branch-and-bound.
   
   Actually, with small n (<=12 buttons) and small m (<=10 counters),
   we can use a straightforward LP approach.
   
   The LP: min 1^T x s.t. Ax = t, x >= 0
   
   Standard form: min c^T x s.t. Ax = b, x >= 0
   
   I'll implement a simple revised simplex or tableau simplex.
   
   Actually, since all target values can be large (up to ~260),
   and we need integer solutions, let me think again.
   
   The key observation: the LP relaxation optimal is a lower bound.
   If the LP gives a non-integer solution, we need B&B.
   
   But given the constraints (m<=10 rows, n<=13 cols), B&B is fine.
   
   I'll implement:
   1. LP relaxation with simplex (big-M or two-phase for feasibility)
   2. If integer, done; else B&B with LP relaxation bounds
   
   Actually, let me try a simpler approach first:
   Since the problem has small n (variables) but potentially large values,
   I'll use a direct ILP solver.
   
   Hmm, but x_j can be up to ~260 each, and n up to 12...
   
   Let me think about the structure more carefully.
   The LP optimal value must be achievable with integers in this specific
   problem structure. Here's why: the LP solution might have fractional
   values, but we can "round" appropriately.
   
   Actually no guarantee. Let me just implement B&B with LP relaxation.
   
   For LP I'll use the two-phase simplex method.
   ============================================================ */

/* Simplex LP solver in standard form:
   min c^T x
   s.t. Ax = b, x >= 0
   b >= 0 (we require this; if b[i] < 0, negate row i)
   
   Returns the optimal objective value or INFINITY if infeasible/unbounded.
   Fills x[0..n-1] with the optimal solution.
*/

#define MAX_VARS 32   /* n + slack variables */
#define MAX_CONS 16   /* number of constraints */
#define INF_VAL 1e30

typedef struct {
    /* Tableau: (m+1) x (n+1) */
    /* Row 0: objective row (z row) */
    /* Rows 1..m: constraint rows */
    /* Last column: RHS */
    int m, n;
    double tab[MAX_CONS + 2][MAX_VARS + 2];
    int basis[MAX_CONS]; /* basis[i] = variable index of basic var in row i+1 */
} Simplex;

static void simplex_init(Simplex *s, int m, int n) {
    s->m = m; s->n = n;
    memset(s->tab, 0, sizeof(s->tab));
}

/* Run simplex iterations until optimal or unbounded.
   Returns 0 if optimal, -1 if unbounded */
static int simplex_iterate(Simplex *s) {
    int m = s->m, n = s->n;
    for (int iter = 0; iter < 10000; iter++) {
        // Find entering variable: most negative reduced cost in row 0
        int enter = -1;
        double min_rc = -1e-9;
        for (int j = 0; j < n; j++) {
            if (s->tab[0][j] < min_rc) {
                min_rc = s->tab[0][j];
                enter = j;
            }
        }
        if (enter < 0) return 0; // optimal
        
        // Find leaving variable: minimum ratio test
        int leave = -1;
        double min_ratio = INF_VAL;
        for (int i = 1; i <= m; i++) {
            if (s->tab[i][enter] > 1e-9) {
                double ratio = s->tab[i][n] / s->tab[i][enter];
                if (ratio < min_ratio - 1e-12) {
                    min_ratio = ratio;
                    leave = i;
                }
            }
        }
        if (leave < 0) return -1; // unbounded
        
        // Pivot
        double pivot = s->tab[leave][enter];
        for (int j = 0; j <= n; j++) s->tab[leave][j] /= pivot;
        for (int i = 0; i <= m; i++) {
            if (i == leave) continue;
            double factor = s->tab[i][enter];
            if (fabs(factor) < 1e-15) continue;
            for (int j = 0; j <= n; j++)
                s->tab[i][j] -= factor * s->tab[leave][j];
        }
        s->basis[leave - 1] = enter;
    }
    return 0; // assume optimal after many iterations
}

/* Solve LP: min c^T x, Ax = b, x >= 0
   n: number of original variables
   m: number of constraints
   A: m x n matrix
   b: m-vector (must be >= 0, adjust sign if needed)
   c: n-vector (objective)
   x_out: output solution (n values)
   Returns objective value, or INF_VAL if infeasible */
static double solve_lp(int m, int n,
                        double A[MAX_CONS][MAX_VARS],
                        double b[MAX_CONS],
                        double c[MAX_VARS],
                        double x_out[MAX_VARS])
{
    /* Two-phase simplex:
       Phase 1: min sum of artificial variables
       Phase 2: min actual objective
       
       We add m artificial variables a_1..a_m.
       Total variables: n + m (original + artificial)
       Phase 1 objective: min sum(a_i)
    */
    int total = n + m; /* variables: x_0..x_{n-1}, a_0..a_{m-1} */
    Simplex s;
    simplex_init(&s, m, total);
    
    /* Set up tableau for phase 1 */
    /* Objective: min sum of artificials (variables n..n+m-1) */
    /* Row 0: objective coefficients */
    for (int j = n; j < total; j++) s.tab[0][j] = 1.0; /* artificial costs */
    
    /* Constraint rows */
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) s.tab[i+1][j] = A[i][j];
        s.tab[i+1][n + i] = 1.0; /* artificial variable */
        s.tab[i+1][total] = b[i];
        s.basis[i] = n + i; /* basis is artificials initially */
    }
    
    /* Update objective row to reflect initial basis (artificials are basic) */
    /* z = sum(a_i), need to eliminate basic variables from objective */
    for (int i = 0; i < m; i++) {
        double factor = s.tab[0][n + i]; /* should be 1.0 */
        if (fabs(factor) < 1e-15) continue;
        for (int j = 0; j <= total; j++)
            s.tab[0][j] -= factor * s.tab[i+1][j];
    }
    
    /* Run phase 1 */
    int ret = simplex_iterate(&s);
    (void)ret;
    
    /* Check feasibility */
    double phase1_obj = -s.tab[0][total]; /* negative because we subtract */
    /* Actually objective is stored as negated sum typically... let me re-check */
    /* With the way we set it up, tab[0][total] should be -phase1_obj */
    /* Wait, the RHS of row 0 starts at 0, and we subtract row contributions */
    /* So -tab[0][total] = phase1_obj */
    
    if (s.tab[0][total] < -1e-6) {
        /* Phase 1 objective < 0 means artificials still nonzero -> infeasible */
        return INF_VAL;
    }
    
    /* Remove artificial variables from basis if present (degenerate case) */
    /* For simplicity, just check if any artificial is in basis with nonzero value */
    /* If phase1_obj ≈ 0, then artificials are at zero, proceed */
    
    /* Phase 2: set up actual objective */
    /* We keep the same tableau but change objective row */
    /* Zero out the objective row first */
    for (int j = 0; j <= total; j++) s.tab[0][j] = 0.0;
    
    /* Set original objective coefficients */
    for (int j = 0; j < n; j++) s.tab[0][j] = c[j];
    
    /* Make artificials very expensive (big-M to keep them out of basis) */
    for (int j = n; j < total; j++) s.tab[0][j] = 1e10;
    
    /* Update objective row to eliminate basic variables */
    for (int i = 0; i < m; i++) {
        int bv = s.basis[i];
        double factor = s.tab[0][bv];
        if (fabs(factor) < 1e-15) continue;
        for (int j = 0; j <= total; j++)
            s.tab[0][j] -= factor * s.tab[i+1][j];
    }
    
    /* Run phase 2 */
    ret = simplex_iterate(&s);
    
    /* Extract solution */
    memset(x_out, 0, n * sizeof(double));
    for (int i = 0; i < m; i++) {
        if (s.basis[i] < n) {
            x_out[s.basis[i]] = s.tab[i+1][total];
        }
    }
    
    /* Objective value: -tab[0][total] + initial_c_dot_zero = -tab[0][total] */
    /* Actually tab[0][total] = -(objective value) since we subtract basic contributions */
    /* Let me verify: if we minimize c^T x, the objective row is z - c^T x = 0 at start */
    /* After pivoting, tab[0][total] = optimal z value (with sign convention) */
    /* Actually with standard simplex: tab[0][total] = z_optimal */
    /* because objective row starts as: z - c^T x = 0, RHS=0 */
    /* after pivots, we have z + (reduced costs)*nonbasics = z_optimal */
    /* so RHS of row 0 = z_optimal */
    
    return s.tab[0][total];
}

/* ============================================================
   Branch and Bound for integer LP (Part 2)
   ============================================================ */

static long long bb_best;

static void branch_and_bound(int m, int n,
                              double A_d[MAX_CONS][MAX_VARS],
                              double b_d[MAX_CONS],
                              double c_d[MAX_VARS],
                              double lb[MAX_VARS],  /* lower bounds on x */
                              double ub[MAX_VARS])  /* upper bounds (INF_VAL if none) */
{
    /* Create modified problem with bounds incorporated as constraints */
    /* Actually, let's add lb[j] <= x[j] <= ub[j] by substitution: x[j] = lb[j] + y[j] */
    /* But this gets complex. Let me use a simple recursive B&B */
    
    /* Adjust b for lower bounds: substitute x[j] = lb[j] + y[j], y[j] >= 0 */
    double b2[MAX_CONS];
    for (int i = 0; i < m; i++) {
        b2[i] = b_d[i];
        for (int j = 0; j < n; j++) b2[i] -= A_d[i][j] * lb[j];
        if (b2[i] < -1e-9) return; /* infeasible */
    }
    
    double x_lp[MAX_VARS];
    double obj = solve_lp(m, n, A_d, b2, c_d, x_lp);
    
    if (obj >= bb_best - 1e-9) return; /* prune: LP bound >= current best */
    if (obj == INF_VAL) return; /* infeasible */
    
    /* Add back lb to get actual x */
    double x_actual[MAX_VARS];
    long long int_sum = 0;
    int frac_idx = -1;
    for (int j = 0; j < n; j++) {
        x_actual[j] = x_lp[j] + lb[j];
        /* Check upper bound */
        if (ub[j] < INF_VAL - 1 && x_actual[j] > ub[j] + 1e-9) return; /* infeasible */
    }
    
    for (int j = 0; j < n; j++) {
        double frac = x_actual[j] - floor(x_actual[j] + 1e-9);
        if (frac > 1e-6 && frac < 1 - 1e-6) {
            frac_idx = j;
            break;
        }
    }
    
    if (frac_idx < 0) {
        /* All integer */
        for (int j = 0; j < n; j++) int_sum += (long long)round(x_actual[j]);
        if (int_sum < bb_best) bb_best = int_sum;
        return;
    }
    
    /* Branch on frac_idx */
    double val = x_actual[frac_idx];
    double floor_val = floor(val + 1e-9);
    double ceil_val = floor_val + 1.0;
    
    /* Branch 1: x[frac_idx] >= ceil_val */
    double new_lb[MAX_VARS], new_ub[MAX_VARS];
    memcpy(new_lb, lb, n * sizeof(double));
    memcpy(new_ub, ub, n * sizeof(double));
    new_lb[frac_idx] = ceil_val;
    branch_and_bound(m, n, A_d, b_d, c_d, new_lb, new_ub);
    
    /* Branch 2: x[frac_idx] <= floor_val */
    memcpy(new_lb, lb, n * sizeof(double));
    memcpy(new_ub, ub, n * sizeof(double));
    new_ub[frac_idx] = floor_val;
    branch_and_bound(m, n, A_d, b_d, c_d, new_lb, new_ub);
}

static long long part2_machine(Machine *m) {
    int n = m->n_buttons;
    int rows = m->n_jolts;
    
    if (rows == 0 || n == 0) return 0;
    
    /* Build A[counter][button] */
    double A_d[MAX_CONS][MAX_VARS];
    double b_d[MAX_CONS];
    double c_d[MAX_VARS];
    memset(A_d, 0, sizeof(A_d));
    
    for (int btn = 0; btn < n; btn++) {
        for (int k = 0; k < m->n_cols[btn]; k++) {
            int ctr = m->cols[btn][k];
            if (ctr < rows) A_d[ctr][btn] = 1.0;
        }
    }
    
    for (int i = 0; i < rows; i++) b_d[i] = (double)m->jolts[i];
    for (int j = 0; j < n; j++) c_d[j] = 1.0; /* minimize sum */
    
    /* Check if b has negatives (it shouldn't, jolts >= 0) */
    for (int i = 0; i < rows; i++) {
        if (b_d[i] < 0) return -1; /* shouldn't happen */
    }
    
    double lb[MAX_VARS], ub[MAX_VARS];
    for (int j = 0; j < n; j++) { lb[j] = 0.0; ub[j] = INF_VAL; }
    
    bb_best = LLONG_MAX;
    branch_and_bound(rows, n, A_d, b_d, c_d, lb, ub);
    
    return bb_best;
}

/* ============================================================
   MAIN
   ============================================================ */

int main(int argc, char *argv[]) {
    int do_part1 = 1, do_part2 = 1;
    if (argc >= 2) {
        if (argv[1][0] == '1') { do_part1 = 1; do_part2 = 0; }
        else if (argv[1][0] == '2') { do_part1 = 0; do_part2 = 1; }
    }
    
    FILE *f = fopen("input.txt", "r");
    if (!f) { fprintf(stderr, "Cannot open input.txt\n"); return 1; }
    
    char line[4096];
    long long sum1 = 0, sum2 = 0;
    
    while (fgets(line, sizeof(line), f)) {
        /* Skip empty lines */
        int has_content = 0;
        for (int i = 0; line[i]; i++) if (line[i] != '\n' && line[i] != '\r' && line[i] != ' ') { has_content = 1; break; }
        if (!has_content) continue;
        
        Machine m;
        if (!parse_line(line, &m)) continue;
        
        if (do_part1) {
            long long v = part1_machine(&m);
            if (v >= 0) sum1 += v;
        }
        if (do_part2) {
            long long v = part2_machine(&m);
            if (v >= 0) sum2 += v;
        }
    }
    
    fclose(f);
    
    if (do_part1) printf("Part 1: %lld\n", sum1);
    if (do_part2) printf("Part 2: %lld\n", sum2);
    
    return 0;
}
