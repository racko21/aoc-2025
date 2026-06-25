import re
import sys
from fractions import Fraction

def parse_input(filename):
    machines = []
    with open(filename) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            lights_match = re.search(r'\[([.\#]+)\]', line)
            lights_str = lights_match.group(1)
            target_lights = [1 if c == '#' else 0 for c in lights_str]
            
            buttons = []
            for m in re.finditer(r'\(([^)]*)\)', line):
                content = m.group(1).strip()
                if content:
                    indices = [int(x) for x in content.split(',')]
                else:
                    indices = []
                buttons.append(indices)
            
            joltage_match = re.search(r'\{([^}]*)\}', line)
            joltage_str = joltage_match.group(1)
            joltage = [int(x) for x in joltage_str.split(',')]
            
            machines.append((target_lights, buttons, joltage))
    return machines

# ==================== PART 1: GF(2) linear algebra ====================

def gf2_solve(A, b, n_vars):
    """
    Solve system Ax = b over GF(2), find minimum weight solution.
    """
    n_eq = len(A)
    
    rows = []
    for i in range(n_eq):
        row = 0
        for j in range(n_vars):
            if A[i][j]:
                row |= (1 << j)
        if b[i]:
            row |= (1 << n_vars)
        rows.append(row)
    
    pivot_cols = []
    pivot_row = {}
    col = 0
    row_idx = 0
    
    while row_idx < len(rows) and col < n_vars:
        found = -1
        for r in range(row_idx, len(rows)):
            if rows[r] & (1 << col):
                found = r
                break
        if found == -1:
            col += 1
            continue
        rows[row_idx], rows[found] = rows[found], rows[row_idx]
        for r in range(len(rows)):
            if r != row_idx and (rows[r] & (1 << col)):
                rows[r] ^= rows[row_idx]
        pivot_cols.append(col)
        pivot_row[col] = row_idx
        row_idx += 1
        col += 1
    
    for r in range(row_idx, len(rows)):
        if rows[r] == (1 << n_vars):
            return None
    
    free_vars = [j for j in range(n_vars) if j not in pivot_row]
    n_free = len(free_vars)
    
    particular = [0] * n_vars
    for pc in pivot_cols:
        ri = pivot_row[pc]
        if rows[ri] & (1 << n_vars):
            particular[pc] = 1
    
    null_basis = []
    for fv in free_vars:
        vec = [0] * n_vars
        vec[fv] = 1
        for pc in pivot_cols:
            ri = pivot_row[pc]
            if rows[ri] & (1 << fv):
                vec[pc] = 1
        null_basis.append(vec)
    
    best = sum(particular)
    
    for mask in range(1, 1 << n_free):
        x = particular[:]
        for k in range(n_free):
            if mask & (1 << k):
                for j in range(n_vars):
                    x[j] ^= null_basis[k][j]
        w = sum(x)
        if w < best:
            best = w
    
    return best

def part1(machines):
    total = 0
    for target_lights, buttons, joltage in machines:
        n_lights = len(target_lights)
        n_buttons = len(buttons)
        
        A = []
        b = []
        for l in range(n_lights):
            row = [0] * n_buttons
            for j, btn in enumerate(buttons):
                if l in btn:
                    row[j] = 1
            A.append(row)
            b.append(target_lights[l])
        
        result = gf2_solve(A, b, n_buttons)
        if result is None:
            total += 0
        else:
            total += result
    return total

# ==================== PART 2: ILP solver ====================
# Minimize sum(x) subject to Ax = b, x >= 0 integers
# We use LP relaxation + branch and bound

def simplex_min(c, A_eq, b_eq, n_vars=None):
    """
    Minimize c^T x subject to A_eq x = b_eq, x >= 0.
    Returns (optimal_value, solution) or None if infeasible/unbounded.
    """
    m = len(A_eq)
    n = len(c) if n_vars is None else n_vars
    
    if m == 0:
        # No constraints, all zero is optimal if c >= 0 and b = []
        return Fraction(0), [Fraction(0)] * n
    
    c = [Fraction(ci) for ci in c]
    A = [[Fraction(A_eq[i][j]) for j in range(n)] for i in range(m)]
    b = [Fraction(b_eq[i]) for i in range(m)]
    
    for i in range(m):
        if b[i] < 0:
            A[i] = [-a for a in A[i]]
            b[i] = -b[i]
    
    n_art = m
    n_total = n + n_art
    
    basis = list(range(n, n + m))
    
    tableau = []
    for i in range(m):
        row = A[i][:] + [Fraction(0)] * m + [b[i]]
        row[n + i] = Fraction(1)
        tableau.append(row)
    
    # Phase 1 objective
    obj_phase1 = [Fraction(0)] * (n_total + 1)
    for j in range(n):
        for i in range(m):
            obj_phase1[j] -= A[i][j]
    for i in range(m):
        obj_phase1[n_total] -= b[i]
    
    def simplex_iterate(tab, obj_row, basis_vars):
        n_cols = len(obj_row) - 1
        for _ in range(100000):
            enter = -1
            min_rc = Fraction(-1, 10**12)
            for j in range(n_cols):
                if obj_row[j] < min_rc:
                    min_rc = obj_row[j]
                    enter = j
            if enter == -1:
                break
            
            leave = -1
            min_ratio = None
            for i in range(len(tab)):
                if tab[i][enter] > 0:
                    ratio = tab[i][n_cols] / tab[i][enter]
                    if min_ratio is None or ratio < min_ratio or \
                       (ratio == min_ratio and basis_vars[i] < basis_vars[leave]):
                        min_ratio = ratio
                        leave = i
            
            if leave == -1:
                return None  # Unbounded
            
            pivot = tab[leave][enter]
            tab[leave] = [x / pivot for x in tab[leave]]
            for i in range(len(tab)):
                if i != leave:
                    factor = tab[i][enter]
                    if factor != 0:
                        tab[i] = [tab[i][k] - factor * tab[leave][k] for k in range(n_cols + 1)]
            factor = obj_row[enter]
            obj_row = [obj_row[k] - factor * tab[leave][k] for k in range(n_cols + 1)]
            basis_vars[leave] = enter
        
        return obj_row, basis_vars
    
    result = simplex_iterate(tableau, obj_phase1, basis)
    if result is None:
        return None
    obj_phase1, basis = result
    
    # Check feasibility (phase 1 optimal should be 0)
    if obj_phase1[n_total] < Fraction(-1, 10**9):
        return None  # Infeasible
    
    # Remove artificials from basis if needed (degenerate case)
    # For simplicity, just proceed to phase 2
    
    # Phase 2: remove artificial columns
    for i in range(m):
        tableau[i] = tableau[i][:n] + [tableau[i][n_total]]
    # Also fix basis: if any artificial is in basis, try to pivot it out
    for i, bv in enumerate(basis):
        if bv >= n:
            # Try to pivot in a non-artificial variable
            pivoted = False
            for j in range(n):
                if tableau[i][j] != 0:
                    # Pivot on (i, j)
                    pivot = tableau[i][j]
                    tableau[i] = [x / pivot for x in tableau[i]]
                    for r in range(m):
                        if r != i:
                            factor = tableau[r][j]
                            if factor != 0:
                                tableau[r] = [tableau[r][k] - factor * tableau[i][k] for k in range(n + 1)]
                    basis[i] = j
                    pivoted = True
                    break
            if not pivoted:
                # Row is all zeros - redundant, mark as degenerate
                pass
    
    obj_phase2 = [Fraction(ci) for ci in c] + [Fraction(0)]
    for i, bv in enumerate(basis):
        if bv < n:
            factor = obj_phase2[bv]
            if factor != 0:
                for k in range(n + 1):
                    obj_phase2[k] -= factor * tableau[i][k]
    
    result2 = simplex_iterate(tableau, obj_phase2, basis)
    if result2 is None:
        return None  # Unbounded (shouldn't happen since sum >= 0)
    obj_phase2, basis = result2
    
    # Extract solution
    sol = [Fraction(0)] * n
    for i, bv in enumerate(basis):
        if bv < n:
            sol[bv] = tableau[i][n]
    
    return -obj_phase2[n], sol

def ilp_min(A_eq, b_eq, n_vars):
    """
    Minimize sum(x) subject to A_eq x = b_eq, x >= 0, x integer.
    Branch and bound with LP relaxation.
    """
    m = len(A_eq)
    c = [1] * n_vars
    
    # Convert to Fraction
    A = [[Fraction(A_eq[i][j]) for j in range(n_vars)] for i in range(m)]
    b = [Fraction(b_eq[i]) for i in range(m)]
    
    best = [None]  # Best integer solution found
    
    # Each node: list of (var, lower_bound, upper_bound) constraints added
    # We represent bounds as lb[j], ub[j] for each variable
    
    def branch_and_bound(lb, ub):
        """lb[j], ub[j] are bounds for variable j (Fraction or None for no bound)"""
        # Solve LP relaxation with bounds
        # Add bounds as constraints: x_j >= lb[j] => x_j - s_j = lb[j]
        # or just use bounds directly
        
        # Build augmented LP: add bound constraints
        # Variables: x_0..x_{n-1}
        # Constraints: A_eq x = b_eq, x_j >= lb_j, x_j <= ub_j
        
        # Simple approach: substitute x_j' = x_j - lb_j when lb_j > 0
        # Then x_j' >= 0, x_j' <= ub_j - lb_j
        # For upper bounds, add slack: x_j' + s_j = ub_j - lb_j, s_j >= 0
        
        # Actually let's just pass bounds to simplex as additional constraints
        # or handle them via big-M or directly
        
        # For simplicity, add upper bound constraints as equality with slack
        # x_j <= ub_j => x_j + s_j = ub_j where s_j >= 0
        
        extra_rows = []
        extra_rhs = []
        n_extra = 0
        
        # Adjust b for lower bounds
        b_adj = b[:]
        for j in range(n_vars):
            if lb[j] > 0:
                for i in range(m):
                    b_adj[i] -= A[i][j] * lb[j]
        
        # Build variable substitution: y_j = x_j - lb_j, y_j >= 0
        # y_j <= ub_j - lb_j if ub_j is finite
        
        # Upper bound constraints: y_j + s_j = ub_j - lb_j
        ub_constrained = []
        for j in range(n_vars):
            if ub[j] is not None:
                span = ub[j] - lb[j]
                if span < 0:
                    return  # Infeasible
                ub_constrained.append((j, span))
        
        n_slack = len(ub_constrained)
        total_vars = n_vars + n_slack
        
        A_full = []
        b_full = []
        
        # Original constraints (adjusted for lb)
        for i in range(m):
            row = list(A[i]) + [Fraction(0)] * n_slack
            A_full.append(row)
            b_full.append(b_adj[i])
        
        # Upper bound constraints
        for k, (j, span) in enumerate(ub_constrained):
            row = [Fraction(0)] * total_vars
            row[j] = Fraction(1)
            row[n_vars + k] = Fraction(1)
            A_full.append(row)
            b_full.append(span)
        
        c_full = [Fraction(1)] * n_vars + [Fraction(0)] * n_slack
        
        result = simplex_min(c_full, A_full, b_full, total_vars)
        if result is None:
            return  # Infeasible
        
        lp_val, sol = result
        # Adjust back: x_j = y_j + lb_j, total = sum y_j + sum lb_j
        lb_sum = sum(lb)
        total_val = lp_val + lb_sum
        
        if best[0] is not None and total_val >= best[0]:
            return  # Prune
        
        # Check if solution is integer
        frac_idx = -1
        frac_val = None
        for j in range(n_vars):
            yj = sol[j]
            xj = yj + lb[j]
            if xj.denominator != 1:
                frac_idx = j
                frac_val = xj
                break
        
        if frac_idx == -1:
            # Integer solution found
            total_int = sum(sol[j] + lb[j] for j in range(n_vars))
            if best[0] is None or total_int < best[0]:
                best[0] = total_int
            return
        
        # Branch on frac_idx
        floor_val = Fraction(int(frac_val))  # floor
        ceil_val = floor_val + 1
        
        # Branch 1: x_{frac_idx} <= floor_val
        new_ub = ub[:]
        new_ub[frac_idx] = floor_val
        branch_and_bound(lb[:], new_ub)
        
        # Branch 2: x_{frac_idx} >= ceil_val
        new_lb = lb[:]
        new_lb[frac_idx] = ceil_val
        branch_and_bound(new_lb, ub[:])
    
    lb = [Fraction(0)] * n_vars
    ub = [None] * n_vars
    # Upper bound: each x_j <= max(b_eq)
    max_b = max(b_eq) if b_eq else 0
    for j in range(n_vars):
        ub[j] = Fraction(max_b)
    
    branch_and_bound(lb, ub)
    
    return best[0]

def part2(machines):
    total = Fraction(0)
    for idx, (target_lights, buttons, joltage) in enumerate(machines):
        n_counters = len(joltage)
        n_buttons = len(buttons)
        
        A = []
        b_vec = []
        for c_idx in range(n_counters):
            row = [0] * n_buttons
            for j, btn in enumerate(buttons):
                if c_idx in btn:
                    row[j] = 1
            A.append(row)
            b_vec.append(joltage[c_idx])
        
        result = ilp_min(A, b_vec, n_buttons)
        if result is None:
            total += 0
        else:
            total += result
    return total

def main():
    machines = parse_input('input.txt')
    
    p1 = part1(machines)
    print(f"Part 1: {p1}")
    
    p2 = part2(machines)
    p2_int = int(p2)
    print(f"Part 2: {p2_int}")

if __name__ == '__main__':
    main()
