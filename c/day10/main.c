#define _POSIX_C_SOURCE 199309L

/* Day 10: Factory. Each machine line lists a target light pattern, a set of
 * buttons, and a joltage vector. Part 1: buttons toggle lights (XOR); find
 * the minimum presses to reach the target light pattern. Part 2: the same
 * buttons instead increment every counter they list by 1 per press; find
 * the minimum total presses to reach the target joltage vector exactly. */

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils/utils.h"

#define MAX_BUTTONS 64
#define MAX_COUNTERS 16

typedef struct {
    unsigned int target;
    int lightCount;
    unsigned int buttons[MAX_BUTTONS];
    int buttonCount;
    long long joltage[MAX_COUNTERS];
    int counterCount;
} Machine;

/* Parses one machine line starting at p into m. Returns a pointer just past
 * the line, or NULL once only trailing whitespace/newlines remain. */
static const char *parse_machine(const char *p, Machine *m) {
    while (*p == ' ' || *p == '\n') {
        p++;
    }
    if (*p == '\0') {
        return NULL;
    }

    m->target = 0;
    m->lightCount = 0;
    m->buttonCount = 0;
    m->counterCount = 0;

    if (*p != '[') {
        fprintf(stderr, "error: expected '[' in machine line\n");
        exit(1);
    }
    p++;
    while (*p != ']') {
        if (*p == '#') {
            m->target |= 1u << m->lightCount;
        }
        m->lightCount++;
        p++;
    }
    p++; /* skip ']' */

    for (;;) {
        while (*p == ' ') {
            p++;
        }
        if (*p == '(') {
            p++;
            unsigned int mask = 0;
            while (*p != ')') {
                int idx;
                int consumed = 0;
                if (sscanf(p, "%d%n", &idx, &consumed) != 1) {
                    fprintf(stderr, "error: malformed button list\n");
                    exit(1);
                }
                mask |= 1u << idx;
                p += consumed;
                if (*p == ',') {
                    p++;
                }
            }
            p++; /* skip ')' */
            if (m->buttonCount >= MAX_BUTTONS) {
                fprintf(stderr, "error: too many buttons\n");
                exit(1);
            }
            m->buttons[m->buttonCount++] = mask;
        } else if (*p == '{') {
            p++;
            while (*p != '}') {
                long long val;
                int consumed = 0;
                if (sscanf(p, "%lld%n", &val, &consumed) != 1) {
                    fprintf(stderr, "error: malformed joltage list\n");
                    exit(1);
                }
                if (m->counterCount >= MAX_COUNTERS) {
                    fprintf(stderr, "error: too many counters\n");
                    exit(1);
                }
                m->joltage[m->counterCount++] = val;
                p += consumed;
                if (*p == ',') {
                    p++;
                }
            }
            p++; /* skip '}' */
            break;
        } else {
            break;
        }
    }

    while (*p == ' ') {
        p++;
    }
    if (*p == '\n') {
        p++;
    }
    return p;
}

/* Brute-forces all 2^buttonCount subsets of the machine's buttons and
 * returns the smallest subset size whose XOR equals the target pattern. */
static int min_presses(const Machine *m) {
    int best = -1;
    unsigned long long subsets = 1ULL << m->buttonCount;
    for (unsigned long long s = 0; s < subsets; s++) {
        unsigned int xorMask = 0;
        int presses = 0;
        for (int i = 0; i < m->buttonCount; i++) {
            if (s & (1ULL << i)) {
                xorMask ^= m->buttons[i];
                presses++;
            }
        }
        if (xorMask == m->target && (best == -1 || presses < best)) {
            best = presses;
        }
    }
    if (best == -1) {
        fprintf(stderr, "error: no button combination reaches target\n");
        exit(1);
    }
    return best;
}

/* ---- Part 2: minimum-weight non-negative integer solution of A x = b ----
 *
 * Pressing button j now ADDS 1 to every counter it lists (instead of XOR),
 * so the joltage target is reached when sum_j x_j * A[:,j] = b exactly,
 * x_j >= 0 integer, minimizing sum_j x_j. This is an integer program: the
 * cheapest integer solution is not necessarily a basic feasible solution of
 * the LP relaxation (it can use more nonzero buttons than the system's
 * rank, e.g. the worked Part 2 example presses 5 distinct buttons to solve
 * a 4-counter system), so a real branch-and-bound over an LP relaxation is
 * required rather than just enumerating rank-sized button combinations.
 *
 * The LP relaxation (min sum x_j s.t. A x = b, x_j >= 0) is solved with a
 * dense Big-M simplex: each equality row gets an artificial variable
 * (cost M), and each currently-imposed per-variable upper bound (from
 * branching) gets its own row with a slack variable (cost 0). Lower bounds
 * from branching are handled by a variable shift (y_j = x_j - lo_j) baked
 * into the right-hand side, so no separate row is needed for them. Branch
 * and bound then tightens lo[]/hi[] on the most fractional variable until
 * every active variable is integral or the relaxed cost can no longer beat
 * the best integer solution found so far. */

#define BIG_M 1.0e7
#define EPS 1.0e-6
#define INF_BOUND -1 /* sentinel: no upper bound imposed yet */

typedef struct {
    double tab[MAX_COUNTERS + MAX_BUTTONS][2 * MAX_BUTTONS + MAX_COUNTERS + 1];
    int basis[MAX_COUNTERS + MAX_BUTTONS];
    int rows, cols;
} Tableau;

/* Runs Big-M simplex (Bland's rule, for guaranteed termination on this tiny
 * problem size) to optimality. Returns 1 if optimal found, 0 if the
 * iteration cap is hit (treated as a solver failure, not infeasibility). */
static int simplex_solve(Tableau *t) {
    int rhsCol = t->cols - 1;
    for (int iter = 0; iter < 20000; iter++) {
        int enter = -1;
        for (int j = 0; j < rhsCol; j++) {
            if (t->tab[t->rows][j] < -EPS) {
                enter = j;
                break; /* Bland's rule: smallest index with negative reduced cost */
            }
        }
        if (enter == -1) {
            return 1; /* optimal */
        }

        int leave = -1;
        double bestRatio = DBL_MAX;
        for (int i = 0; i < t->rows; i++) {
            if (t->tab[i][enter] > EPS) {
                double ratio = t->tab[i][rhsCol] / t->tab[i][enter];
                if (ratio < bestRatio - EPS ||
                    (ratio < bestRatio + EPS && (leave == -1 || t->basis[i] < t->basis[leave]))) {
                    bestRatio = ratio;
                    leave = i;
                }
            }
        }
        if (leave == -1) {
            return 0; /* unbounded: should not happen, all costs are >= 0 */
        }

        double pivot = t->tab[leave][enter];
        for (int j = 0; j <= rhsCol; j++) {
            t->tab[leave][j] /= pivot;
        }
        for (int i = 0; i <= t->rows; i++) {
            if (i == leave) {
                continue;
            }
            double factor = t->tab[i][enter];
            if (factor == 0.0) {
                continue;
            }
            for (int j = 0; j <= rhsCol; j++) {
                t->tab[i][j] -= factor * t->tab[leave][j];
            }
        }
        t->basis[leave] = enter;
    }
    return 0;
}

/* Solves the LP relaxation of the machine's joltage system under the
 * current per-button [lo, hi] bounds (hi[j] == INF_BOUND means unbounded
 * above). On success, writes the relaxed optimal x_j into outX and the
 * total cost into outCost. Returns 1 if feasible, 0 if infeasible. */
static int solve_lp(const Machine *m, const long long *lo, const long long *hi, double *outX,
                     double *outCost) {
    int C = m->counterCount;
    int B = m->buttonCount;

    double b[MAX_COUNTERS];
    for (int i = 0; i < C; i++) {
        double bi = (double)m->joltage[i];
        for (int j = 0; j < B; j++) {
            if (m->buttons[j] & (1u << i)) {
                bi -= (double)lo[j];
            }
        }
        if (bi < -EPS) {
            return 0; /* lower bounds alone already overshoot this counter */
        }
        b[i] = bi < 0 ? 0 : bi;
    }

    int active[MAX_BUTTONS];
    double ub[MAX_BUTTONS];
    int nActive = 0;
    for (int j = 0; j < B; j++) {
        if (hi[j] != INF_BOUND) {
            double width = (double)(hi[j] - lo[j]);
            if (width < -EPS) {
                return 0; /* lo > hi: branch is infeasible */
            }
            if (width <= EPS) {
                continue; /* fixed at lo[j], excluded from the LP entirely */
            }
            ub[nActive] = width;
        } else {
            ub[nActive] = -1; /* unbounded */
        }
        active[nActive++] = j;
    }

    int nSlackRows = 0;
    for (int k = 0; k < nActive; k++) {
        if (ub[k] >= 0) {
            nSlackRows++;
        }
    }

    Tableau t;
    t.rows = C + nSlackRows;
    t.cols = nActive + nSlackRows + C + 1;
    int rhsCol = t.cols - 1;
    for (int i = 0; i <= t.rows; i++) {
        for (int j = 0; j <= rhsCol; j++) {
            t.tab[i][j] = 0.0;
        }
    }

    for (int i = 0; i < C; i++) {
        for (int k = 0; k < nActive; k++) {
            if (m->buttons[active[k]] & (1u << i)) {
                t.tab[i][k] = 1.0;
            }
        }
        t.tab[i][nActive + nSlackRows + i] = 1.0; /* artificial */
        t.tab[i][rhsCol] = b[i];
        t.basis[i] = nActive + nSlackRows + i;
    }

    int row = C;
    for (int k = 0; k < nActive; k++) {
        if (ub[k] >= 0) {
            t.tab[row][k] = 1.0;
            t.tab[row][nActive + (row - C)] = 1.0; /* slack */
            t.tab[row][rhsCol] = ub[k];
            t.basis[row] = nActive + (row - C);
            row++;
        }
    }

    /* Objective row: cost 1 per active variable, BIG_M per artificial,
     * expressed as -cost since the simplex above looks for negative
     * reduced costs, then reduced for the variables already basic. */
    for (int k = 0; k < nActive; k++) {
        t.tab[t.rows][k] = 1.0;
    }
    for (int i = 0; i < C; i++) {
        t.tab[t.rows][nActive + nSlackRows + i] = BIG_M;
    }
    for (int i = 0; i < t.rows; i++) {
        double c = t.tab[t.rows][t.basis[i]];
        if (c != 0.0) {
            for (int j = 0; j <= rhsCol; j++) {
                t.tab[t.rows][j] -= c * t.tab[i][j];
            }
        }
    }

    if (!simplex_solve(&t)) {
        return 0;
    }

    /* Any artificial variable left basic with a positive value means the
     * equality system could not be satisfied under these bounds. */
    for (int i = 0; i < C; i++) {
        if (t.basis[i] >= nActive + nSlackRows) {
            if (t.tab[i][rhsCol] > EPS) {
                return 0;
            }
        }
    }

    double yVal[MAX_BUTTONS];
    for (int k = 0; k < nActive; k++) {
        yVal[k] = 0.0;
    }
    for (int i = 0; i < t.rows; i++) {
        if (t.basis[i] < nActive) {
            yVal[t.basis[i]] = t.tab[i][rhsCol];
        }
    }

    double total = 0.0;
    for (int j = 0; j < B; j++) {
        outX[j] = (double)lo[j];
        total += (double)lo[j];
    }
    for (int k = 0; k < nActive; k++) {
        outX[active[k]] += yVal[k];
        total += yVal[k];
    }
    *outCost = total;
    return 1;
}

/* Floor for the non-negative, bounded press counts this solver produces;
 * avoids pulling in libm (the shared Makefile doesn't link -lm). */
static long long floor_nonneg(double v) {
    return (long long)v;
}

static long long g_best;

/* Recursively branches on the most fractional button's press count until
 * an integer-feasible solution is found or the LP bound can no longer beat
 * the best integer solution seen so far. */
static void branch_and_bound(const Machine *m, long long *lo, long long *hi) {
    double x[MAX_BUTTONS];
    double cost;
    if (!solve_lp(m, lo, hi, x, &cost)) {
        return;
    }
    if (cost >= (double)g_best - EPS) {
        return; /* relaxed lower bound cannot beat the incumbent */
    }

    int frac = -1;
    double bestDist = EPS;
    for (int j = 0; j < m->buttonCount; j++) {
        double f = x[j] - (double)floor_nonneg(x[j] + EPS);
        double dist = f < 0.5 ? f : 1.0 - f;
        if (dist > bestDist) {
            bestDist = dist;
            frac = j;
        }
    }

    if (frac == -1) {
        long long total = 0;
        for (int j = 0; j < m->buttonCount; j++) {
            total += (long long)(x[j] + 0.5);
        }
        if (total < g_best) {
            g_best = total;
        }
        return;
    }

    long long floorVal = floor_nonneg(x[frac] + EPS);
    long long ceilVal = floorVal + 1;

    long long savedHi = hi[frac];
    hi[frac] = floorVal;
    branch_and_bound(m, lo, hi);
    hi[frac] = savedHi;

    long long savedLo = lo[frac];
    lo[frac] = ceilVal;
    branch_and_bound(m, lo, hi);
    lo[frac] = savedLo;
}

/* Returns the minimum total button presses to reach the machine's exact
 * joltage target via branch-and-bound over the LP relaxation. */
static long long min_joltage_presses(const Machine *m) {
    long long lo[MAX_BUTTONS];
    long long hi[MAX_BUTTONS];
    for (int j = 0; j < m->buttonCount; j++) {
        lo[j] = 0;
        hi[j] = INF_BOUND;
    }
    g_best = (long long)1 << 50;
    branch_and_bound(m, lo, hi);
    if (g_best == (long long)1 << 50) {
        fprintf(stderr, "error: no integer solution reaches joltage target\n");
        exit(1);
    }
    return g_best;
}

long long solve_part2(const char *path) {
    char *buf = read_file(path);
    long long total = 0;
    const char *p = buf;
    Machine m;
    while ((p = parse_machine(p, &m)) != NULL) {
        if (m.counterCount != m.lightCount) {
            fprintf(stderr, "error: joltage count does not match light count\n");
            exit(1);
        }
        total += min_joltage_presses(&m);
    }
    free(buf);
    return total;
}

long long solve_part1(const char *path) {
    char *buf = read_file(path);
    long long total = 0;
    const char *p = buf;
    Machine m;
    while ((p = parse_machine(p, &m)) != NULL) {
        total += min_presses(&m);
    }
    free(buf);
    return total;
}

#ifndef UNIT_TEST
static double elapsed_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000.0 + (b.tv_nsec - a.tv_nsec) / 1e6;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc > 1) {
        part = atoi(argv[1]);
    }

    struct timespec t0, t1;
    if (part == 0 || part == 1) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans1 = solve_part1("day10/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("day10/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }
    return 0;
}
#endif
