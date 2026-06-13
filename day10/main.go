// Day 10: minimum button presses for indicator lights (Part 1) and joltage counters (Part 2).
// Part 1: BFS over 2^N XOR-toggle light states.
// Part 2: ILP via branch-and-bound with Big-M LP relaxation at each node.
//   min Σ x_j  s.t.  A x = b,  x ≥ 0,  x ∈ ℤ
//   A[i][j] = 1 iff button j increments counter i.
package main

import (
	"fmt"
	"math"
	"regexp"
	"strconv"
	"strings"

	"github.com/racko21/aoc-2025/utils"
)

var (
	reTarget = regexp.MustCompile(`\[([^\]]+)\]`)
	reButton = regexp.MustCompile(`\(([^)]*)\)`)
	reJolt   = regexp.MustCompile(`\{([^}]*)\}`)
)

// parseLine extracts light diagram, buttons (bitmasks + index lists), and joltage targets.
func parseLine(line string) (lightTarget int, lightButtons []int, joltTargets []int, buttonSets [][]int) {
	if tm := reTarget.FindStringSubmatch(line); tm != nil {
		for i, ch := range tm[1] {
			if ch == '#' {
				lightTarget |= 1 << i
			}
		}
	}
	for _, bm := range reButton.FindAllStringSubmatch(line, -1) {
		mask := 0
		var indices []int
		for _, s := range strings.Split(bm[1], ",") {
			s = strings.TrimSpace(s)
			if s == "" {
				continue
			}
			idx, _ := strconv.Atoi(s)
			mask |= 1 << idx
			indices = append(indices, idx)
		}
		if mask != 0 {
			lightButtons = append(lightButtons, mask)
			buttonSets = append(buttonSets, indices)
		}
	}
	if jm := reJolt.FindStringSubmatch(line); jm != nil {
		for _, s := range strings.Split(jm[1], ",") {
			s = strings.TrimSpace(s)
			if s == "" {
				continue
			}
			v, _ := strconv.Atoi(s)
			joltTargets = append(joltTargets, v)
		}
	}
	return
}

// minPresses returns fewest button presses to reach target light configuration via BFS.
func minPresses(target int, buttons []int) int {
	if target == 0 {
		return 0
	}
	dist := map[int]int{0: 0}
	queue := []int{0}
	for len(queue) > 0 {
		cur := queue[0]
		queue = queue[1:]
		d := dist[cur]
		for _, btn := range buttons {
			next := cur ^ btn
			if _, seen := dist[next]; !seen {
				if next == target {
					return d + 1
				}
				dist[next] = d + 1
				queue = append(queue, next)
			}
		}
	}
	return -1
}

const lpEps = 1e-9

type lpSol struct {
	val float64   // optimal value; -1 = infeasible; +Inf = unbounded
	x   []float64 // variable values (nil if infeasible/unbounded)
}

// pivot performs a simplex pivot: brings enterCol into the basis in leaveRow.
func pivot(tab [][]float64, basis []int, leaveRow, enterCol int) {
	cols := len(tab[0])
	m := len(basis)
	basis[leaveRow-1] = enterCol
	piv := tab[leaveRow][enterCol]
	for j := 0; j < cols; j++ {
		tab[leaveRow][j] /= piv
	}
	for i := 0; i <= m; i++ {
		if i == leaveRow {
			continue
		}
		f := tab[i][enterCol]
		if f == 0 {
			continue
		}
		for j := 0; j < cols; j++ {
			tab[i][j] -= f * tab[leaveRow][j]
		}
	}
}

// simplexPhase runs the simplex method (Bland's rule) considering entering
// candidates in columns [0, maxEnter). Returns false if unbounded.
func simplexPhase(tab [][]float64, basis []int, maxEnter int) bool {
	m := len(basis)
	cols := len(tab[0])
	for iter := 0; iter < 500000; iter++ {
		enterCol := -1
		for j := 0; j < maxEnter; j++ {
			if tab[0][j] < -lpEps {
				enterCol = j
				break
			}
		}
		if enterCol == -1 {
			return true
		}
		leaveRow := -1
		minRatio := math.Inf(1)
		for i := 1; i <= m; i++ {
			if tab[i][enterCol] > lpEps {
				ratio := tab[i][cols-1] / tab[i][enterCol]
				if ratio < minRatio-lpEps || (ratio < minRatio+lpEps && leaveRow != -1 && basis[i-1] < basis[leaveRow-1]) {
					minRatio = ratio
					leaveRow = i
				}
			}
		}
		if leaveRow == -1 {
			return false // unbounded
		}
		pivot(tab, basis, leaveRow, enterCol)
	}
	return true
}

// lpSolve solves:
//   min Σ x[0..n-1]  (original variables only)
//   s.t. Aeq * x = beq           (equality constraints, mEq rows, all beq ≥ 0)
//        x[ubIdx[k]] ≤ ubVal[k]  (upper bound constraints as inequalities)
//        x ≥ 0
// Uses two-phase simplex with slack variables for upper bounds.
// Returns lpSol{val, x[0..n-1]} or {-1,nil} if infeasible.
func lpSolve(Aeq [][]float64, beq []float64, ubIdx []int, ubVal []float64) lpSol {
	mEq := len(beq)
	numUB := len(ubIdx)
	if mEq == 0 && numUB == 0 {
		return lpSol{0, nil}
	}
	n := 0
	if mEq > 0 {
		n = len(Aeq[0])
	} else if numUB > 0 {
		// Find max index
		for _, j := range ubIdx {
			if j+1 > n {
				n = j + 1
			}
		}
	}
	if n == 0 {
		for _, bi := range beq {
			if bi > lpEps {
				return lpSol{-1, nil}
			}
		}
		return lpSol{0, nil}
	}

	// Layout: 0..n-1 = originals, n..n+numUB-1 = UB slacks, n+numUB..n+numUB+mEq-1 = artificials.
	nSlk := numUB
	nArt := mEq
	totalM := mEq + numUB
	cols := n + nSlk + nArt + 1

	tab := make([][]float64, totalM+1)
	for i := range tab {
		tab[i] = make([]float64, cols)
	}
	basis := make([]int, totalM)

	// Equality rows: artificial in basis.
	for i := 0; i < mEq; i++ {
		for j := 0; j < n; j++ {
			tab[i+1][j] = Aeq[i][j]
		}
		tab[i+1][n+nSlk+i] = 1.0
		tab[i+1][cols-1] = beq[i]
		basis[i] = n + nSlk + i
	}

	// Upper-bound rows: slack in basis (no artificial needed since UB RHS ≥ 0).
	for k, j := range ubIdx {
		tab[mEq+k+1][j] = 1.0
		tab[mEq+k+1][n+k] = 1.0
		tab[mEq+k+1][cols-1] = ubVal[k]
		basis[mEq+k] = n + k
	}

	// Phase 1 objective: min Σ a_i (artificials only; slacks cost 0).
	// With initial basis {art_i, slk_k}:
	//   c̄_{x_j} = -Σ_{eq row i} Aeq[i][j]   (only eq rows contribute)
	//   c̄_{slk_k} = 0  (in basis)
	//   c̄_{art_i} = 0  (in basis)
	//   RHS = -Σ beq[i]
	for j := 0; j < n; j++ {
		var cs float64
		for i := 0; i < mEq; i++ {
			cs += Aeq[i][j]
		}
		tab[0][j] = -cs
	}
	for i := 0; i < mEq; i++ {
		tab[0][cols-1] -= beq[i]
	}

	// Phase 1: all variables eligible (originals + slacks + artificials).
	simplexPhase(tab, basis, n+nSlk+nArt)

	// Feasibility: no artificial may remain with non-zero value.
	for i, bv := range basis {
		if bv >= n+nSlk && tab[i+1][cols-1] > lpEps {
			return lpSol{-1, nil}
		}
	}

	// Drive out degenerate artificials (value = 0) before Phase 2.
	// Replace each with any original/slack variable that has non-zero coefficient.
	for changed := true; changed; {
		changed = false
		for i, bv := range basis {
			if bv < n+nSlk {
				continue
			}
			// Artificial at 0 — try to replace.
			for j := 0; j < n+nSlk; j++ {
				if math.Abs(tab[i+1][j]) > lpEps {
					pivot(tab, basis, i+1, j)
					changed = true
					break
				}
			}
		}
	}

	// Phase 2: min Σ x[0..n-1]; slacks have cost 0; block artificials.
	for j := 0; j < cols; j++ {
		tab[0][j] = 0
	}
	for j := 0; j < n; j++ {
		tab[0][j] = 1.0
	}
	// Eliminate basic originals from Phase 2 objective row.
	for i, bv := range basis {
		if bv < n {
			f := tab[0][bv]
			if f != 0 {
				for j := 0; j < cols; j++ {
					tab[0][j] -= f * tab[i+1][j]
				}
			}
		}
	}
	// Block artificials (they carry no Phase 2 benefit).
	for j := n + nSlk; j < n+nSlk+nArt; j++ {
		tab[0][j] = math.Abs(tab[0][j]) + 1e8
	}

	// Phase 2: originals and UB slacks can enter (artificials blocked).
	if !simplexPhase(tab, basis, n+nSlk) {
		return lpSol{math.Inf(1), nil}
	}

	x := make([]float64, n)
	for i, bv := range basis {
		if bv < n {
			x[bv] = tab[i+1][cols-1]
		}
	}
	return lpSol{-tab[0][cols-1], x}
}

// bnb runs branch-and-bound to solve min{Σ y_j : Ay = b, lb ≤ y ≤ ub, y integer}.
// b is the original (unshifted) RHS. best is updated on any improvement.
func bnb(A [][]float64, b []float64, lb, ub []float64, best *float64) {
	n := len(lb)
	m := len(b)

	// Infeasible if any lb > ub.
	for j := 0; j < n; j++ {
		if !math.IsInf(ub[j], 1) && ub[j] < lb[j]-lpEps {
			return
		}
	}

	var lbSum float64
	for _, l := range lb {
		lbSum += l
	}

	// Shift variables: x = y − lb; new equality RHS = b − A·lb.
	b2 := make([]float64, m)
	copy(b2, b)
	for j := 0; j < n; j++ {
		if lb[j] != 0 {
			for i := 0; i < m; i++ {
				b2[i] -= A[i][j] * lb[j]
			}
		}
	}
	for _, bi := range b2 {
		if bi < -lpEps {
			return // shifted RHS negative → infeasible
		}
	}

	// Build UB index/value lists for the shifted problem: x[j] ≤ ub[j] − lb[j].
	var ubIdx []int
	var ubVal []float64
	for j := 0; j < n; j++ {
		if !math.IsInf(ub[j], 1) {
			ubIdx = append(ubIdx, j)
			ubVal = append(ubVal, ub[j]-lb[j])
		}
	}

	sol := lpSolve(A, b2, ubIdx, ubVal)
	if sol.val < -lpEps || math.IsInf(sol.val, 1) {
		return
	}

	// Clamp tiny-negative LP values (floating-point artefacts).
	for j := 0; j < n; j++ {
		if sol.x[j] < 0 {
			sol.x[j] = 0
		}
	}

	totalVal := sol.val + lbSum
	if totalVal >= *best-lpEps {
		return
	}

	// Find most-fractional variable (closest to 0.5).
	branchVar, bestDist := -1, math.Inf(1)
	for j := 0; j < n; j++ {
		frac := sol.x[j] - math.Floor(sol.x[j])
		if frac > lpEps && frac < 1-lpEps {
			if d := math.Abs(frac - 0.5); d < bestDist {
				bestDist = d
				branchVar = j
			}
		}
	}
	if branchVar == -1 {
		// Verify: compute y and check Ay = b exactly.
		y := make([]float64, n)
		var ySum float64
		for j := 0; j < n; j++ {
			y[j] = math.Round(sol.x[j] + lb[j])
			ySum += y[j]
		}
		for i := 0; i < m; i++ {
			var sum float64
			for j := 0; j < n; j++ {
				sum += A[i][j] * y[j]
			}
			if math.Abs(sum-b[i]) > 0.5 {
				return // LP gave invalid integer solution; skip
			}
		}
		*best = ySum
		return
	}

	yj := sol.x[branchVar] + lb[branchVar]
	floorY := math.Floor(yj + lpEps)
	ceilY := math.Ceil(yj - lpEps)

	// Branch down: y[branchVar] ≤ floorY.
	if floorY < ub[branchVar]-lpEps {
		ub0 := make([]float64, n)
		copy(ub0, ub)
		ub0[branchVar] = floorY
		bnb(A, b, lb, ub0, best)
	}

	// Branch up: y[branchVar] ≥ ceilY.
	if ceilY > lb[branchVar]+lpEps {
		lb1 := make([]float64, n)
		copy(lb1, lb)
		lb1[branchVar] = ceilY
		bnb(A, b, lb1, ub, best)
	}
}

// solveJoltage finds minimum total button presses to set each counter to its target.
func solveJoltage(buttonSets [][]int, targets []int) int64 {
	m := len(targets)
	n := len(buttonSets)
	if m == 0 {
		return 0
	}

	A := make([][]float64, m)
	for i := range A {
		A[i] = make([]float64, n)
	}
	for j, indices := range buttonSets {
		for _, idx := range indices {
			if idx < m {
				A[idx][j] = 1.0
			}
		}
	}
	b := make([]float64, m)
	for i, t := range targets {
		b[i] = float64(t)
	}

	lb := make([]float64, n)
	ub := make([]float64, n)
	for i := range ub {
		ub[i] = math.Inf(1)
	}
	best := math.Inf(1)
	bnb(A, b, lb, ub, &best)

	if math.IsInf(best, 1) {
		return -1
	}
	return int64(math.Round(best))
}

func part1(path string) int {
	total := 0
	for _, line := range utils.ReadLines(path) {
		if line == "" {
			continue
		}
		target, buttons, _, _ := parseLine(line)
		total += minPresses(target, buttons)
	}
	return total
}

func part2(path string) int64 {
	var total int64
	for _, line := range utils.ReadLines(path) {
		if line == "" {
			continue
		}
		_, _, joltTargets, buttonSets := parseLine(line)
		total += solveJoltage(buttonSets, joltTargets)
	}
	return total
}

func main() {
	fmt.Println("Part 1:", part1("input.txt"))
	fmt.Println("Part 2:", part2("input.txt"))
}
