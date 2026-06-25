package main

import (
	"bufio"
	"fmt"
	"math"
	"os"
	"strconv"
	"strings"
)

// ---- Parsing ----

type Machine struct {
	lights  []bool  // target light pattern
	buttons [][]int // button -> list of indices it toggles
	joltage []int   // target joltage requirements
}

func parseMachine(line string) Machine {
	var m Machine

	lStart := strings.Index(line, "[")
	lEnd := strings.Index(line, "]")
	lightStr := line[lStart+1 : lEnd]
	for _, c := range lightStr {
		m.lights = append(m.lights, c == '#')
	}

	rest := line[lEnd+1:]
	for {
		bStart := strings.Index(rest, "(")
		if bStart == -1 {
			break
		}
		bEnd := strings.Index(rest, ")")
		btnStr := rest[bStart+1 : bEnd]
		rest = rest[bEnd+1:]
		var btn []int
		for _, s := range strings.Split(btnStr, ",") {
			s = strings.TrimSpace(s)
			if s == "" {
				continue
			}
			n, _ := strconv.Atoi(s)
			btn = append(btn, n)
		}
		m.buttons = append(m.buttons, btn)
	}

	jStart := strings.Index(line, "{")
	jEnd := strings.Index(line, "}")
	joltStr := line[jStart+1 : jEnd]
	for _, s := range strings.Split(joltStr, ",") {
		s = strings.TrimSpace(s)
		n, _ := strconv.Atoi(s)
		m.joltage = append(m.joltage, n)
	}

	return m
}

func readMachines(filename string) []Machine {
	f, err := os.Open(filename)
	if err != nil {
		panic(err)
	}
	defer f.Close()

	var machines []Machine
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		machines = append(machines, parseMachine(line))
	}
	return machines
}

// ---- Part 1: GF(2) min weight ----

func solvePart1Machine(m Machine) int {
	n := len(m.buttons)
	nLights := len(m.lights)

	target := 0
	for i, on := range m.lights {
		if on {
			target |= 1 << i
		}
	}

	effects := make([]int, n)
	for i, btn := range m.buttons {
		for _, idx := range btn {
			if idx < nLights {
				effects[i] |= 1 << idx
			}
		}
	}

	best := math.MaxInt64
	for mask := 0; mask < (1 << n); mask++ {
		result := 0
		cnt := 0
		for i := 0; i < n; i++ {
			if mask&(1<<i) != 0 {
				result ^= effects[i]
				cnt++
			}
		}
		if result == target && cnt < best {
			best = cnt
		}
	}
	return best
}

func solvePart1(machines []Machine) int {
	total := 0
	for _, m := range machines {
		total += solvePart1Machine(m)
	}
	return total
}

// ---- Part 2: LP relaxation + Branch and Bound ----
//
// Minimize sum(x_i) s.t. Ax = b, x_i >= 0, x_i integer
//
// Two-phase simplex: 
//   Phase 1: find a basic feasible solution by minimizing sum of artificials
//   Phase 2: minimize actual objective from that BFS
//
// Convention for tableau:
//   obj[j] = reduced cost of variable j (we want all >= 0 for min)
//   obj[N] = -z (negative of current objective value)
//   After optimal: optVal = -obj[N]

const EPS = 1e-9
const BIGCOST = 1e12 // large cost for artificials in phase 2

// twoPhase solves: min c^T x, Ax = b, x >= 0
// Assumes b >= 0 (caller ensures).
// n = number of real variables (0..n-1); artificials are appended internally.
// Returns (opt, x[0..n-1], feasible).
func twoPhase(Aorig [][]float64, borig []float64, corig []float64) (float64, []float64, bool) {
	m := len(Aorig)
	n := len(corig)
	if m == 0 {
		return 0, make([]float64, n), true
	}

	// Deep copy and ensure b >= 0
	A := make([][]float64, m)
	b := make([]float64, m)
	for i := 0; i < m; i++ {
		A[i] = make([]float64, n)
		copy(A[i], Aorig[i])
		b[i] = borig[i]
	}
	for i := 0; i < m; i++ {
		if b[i] < -EPS {
			b[i] = -b[i]
			for j := 0; j < n; j++ {
				A[i][j] = -A[i][j]
			}
		}
	}

	// Variables: 0..n-1 (real), n..n+m-1 (artificials)
	N := n + m // number of variables

	// tab[i][j]: coefficient of var j in constraint i
	// tab[i][N]: RHS of constraint i
	tab := make([][]float64, m)
	for i := 0; i < m; i++ {
		tab[i] = make([]float64, N+1)
		for j := 0; j < n; j++ {
			tab[i][j] = A[i][j]
		}
		tab[i][n+i] = 1.0
		tab[i][N] = b[i]
	}

	basis := make([]int, m)
	for i := 0; i < m; i++ {
		basis[i] = n + i
	}

	// Phase 1: minimize sum of artificials
	// obj[j] = reduced cost of var j; obj[N] = -z
	obj1 := make([]float64, N+1)
	for i := 0; i < m; i++ {
		obj1[n+i] = 1.0 // artificial i has cost 1
	}
	// obj[N] = -z_initial = -sum(b[i]) (since artificials are basic = b[i])
	// Eliminate basics from objective:
	for i := 0; i < m; i++ {
		// basic var n+i, cost = obj1[n+i] = 1
		coef := obj1[n+i]
		for j := 0; j <= N; j++ {
			obj1[j] -= coef * tab[i][j]
		}
	}
	// Now obj1 is set up properly

	simplexIterate(tab, basis, obj1, m, N, N) // allow all vars

	z1 := -obj1[N]
	if z1 > EPS {
		return 0, nil, false // infeasible
	}

	// Drive artificials out of basis
	for i := 0; i < m; i++ {
		if basis[i] >= n {
			pivoted := false
			for j := 0; j < n; j++ {
				if math.Abs(tab[i][j]) > EPS {
					simplexPivot(tab, basis, obj1, i, j, m, N)
					pivoted = true
					break
				}
			}
			_ = pivoted
		}
	}

	// Phase 2: minimize actual objective
	// Only allow real variables (0..n-1) to enter
	obj2 := make([]float64, N+1)
	for j := 0; j < n; j++ {
		obj2[j] = corig[j]
	}
	// Artificials: give them large cost so they won't enter
	for i := 0; i < m; i++ {
		obj2[n+i] = BIGCOST
	}
	// Eliminate basic real variables from obj2
	for i := 0; i < m; i++ {
		if basis[i] < n {
			coef := obj2[basis[i]]
			if math.Abs(coef) > EPS {
				for j := 0; j <= N; j++ {
					obj2[j] -= coef * tab[i][j]
				}
			}
		}
	}

	simplexIterate(tab, basis, obj2, m, N, n) // only allow vars 0..n-1

	optVal := -obj2[N]
	x := make([]float64, n)
	for i := 0; i < m; i++ {
		if basis[i] < n {
			x[basis[i]] = tab[i][N]
		}
	}
	return optVal, x, true
}

// simplexIterate runs the simplex algorithm.
// maxEnter: only consider variables 0..maxEnter-1 as entering.
func simplexIterate(tab [][]float64, basis []int, obj []float64, m, N, maxEnter int) {
	for iter := 0; iter < 500000; iter++ {
		enterCol := -1
		minVal := -EPS
		for j := 0; j < maxEnter; j++ {
			if obj[j] < minVal {
				minVal = obj[j]
				enterCol = j
			}
		}
		if enterCol == -1 {
			break
		}

		leaveRow := -1
		minRatio := math.Inf(1)
		for i := 0; i < m; i++ {
			if tab[i][enterCol] > EPS {
				ratio := tab[i][N] / tab[i][enterCol]
				if ratio < minRatio-EPS {
					minRatio = ratio
					leaveRow = i
				} else if math.Abs(ratio-minRatio) < EPS && leaveRow >= 0 && basis[i] < basis[leaveRow] {
					leaveRow = i
				}
			}
		}
		if leaveRow == -1 {
			break // unbounded
		}

		simplexPivot(tab, basis, obj, leaveRow, enterCol, m, N)
	}
}

func simplexPivot(tab [][]float64, basis []int, obj []float64, row, col, m, N int) {
	pivVal := tab[row][col]
	for j := 0; j <= N; j++ {
		tab[row][j] /= pivVal
	}
	for i := 0; i < m; i++ {
		if i == row {
			continue
		}
		factor := tab[i][col]
		if math.Abs(factor) < EPS {
			continue
		}
		for j := 0; j <= N; j++ {
			tab[i][j] -= factor * tab[row][j]
		}
	}
	factor := obj[col]
	if math.Abs(factor) > EPS {
		for j := 0; j <= N; j++ {
			obj[j] -= factor * tab[row][j]
		}
	}
	basis[row] = col
}

// solveLP solves: min sum(x_i) s.t. Ax=b, lb[i]<=x[i]<=ub[i], x>=0
// The "shift" approach: let y_i = x_i - lb_i >= 0, y_i <= ub_i - lb_i
// Then Ay = b - A*lb, minimize sum(y_i) + sum(lb_i)
// Additional constraints: y_i + s_i = ub_i - lb_i (s_i >= 0)
// Returns (opt value of sum x_i, y solution, feasible)
func solveLP(A [][]float64, b []float64, n int, lb, ub []float64) (float64, []float64, bool) {
	m := len(b)

	bShift := make([]float64, m+n)
	for j := 0; j < m; j++ {
		bShift[j] = b[j]
		for i := 0; i < n; i++ {
			bShift[j] -= A[j][i] * lb[i]
		}
		if bShift[j] < -EPS {
			return 0, nil, false // infeasible
		}
	}

	for i := 0; i < n; i++ {
		ubShift := ub[i] - lb[i]
		if ubShift < -EPS {
			return 0, nil, false
		}
		bShift[m+i] = ubShift
	}

	// Build extended A: m+n rows, 2n cols
	AExt := make([][]float64, m+n)
	for j := 0; j < m; j++ {
		AExt[j] = make([]float64, 2*n)
		for i := 0; i < n; i++ {
			AExt[j][i] = A[j][i]
		}
	}
	for i := 0; i < n; i++ {
		AExt[m+i] = make([]float64, 2*n)
		AExt[m+i][i] = 1.0
		AExt[m+i][n+i] = 1.0
	}

	cExt := make([]float64, 2*n)
	for i := 0; i < n; i++ {
		cExt[i] = 1.0 // minimize sum of y_i (original vars)
	}

	lpVal, ySol, ok := twoPhase(AExt, bShift, cExt)
	if !ok {
		return 0, nil, false
	}

	lbSum := 0.0
	for i := 0; i < n; i++ {
		lbSum += lb[i]
	}

	return lpVal + lbSum, ySol, true
}

// solveILP uses branch and bound to find minimum integer solution.
func solveILP(A [][]float64, b []float64, n int) (int64, bool) {
	m := len(b)
	_ = m

	// Compute a reasonable upper bound on each x_i
	maxB := 0.0
	for _, bj := range b {
		if bj > maxB {
			maxB = bj
		}
	}

	type Node struct {
		lb []float64
		ub []float64
	}

	initLB := make([]float64, n)
	initUB := make([]float64, n)
	for i := range initUB {
		initUB[i] = maxB
	}

	var bestVal int64 = math.MaxInt64
	stack := []Node{{lb: initLB, ub: initUB}}

	for len(stack) > 0 {
		node := stack[len(stack)-1]
		stack = stack[:len(stack)-1]

		lpVal, ySol, ok := solveLP(A, b, n, node.lb, node.ub)
		if !ok {
			continue
		}

		if lpVal > float64(bestVal)-0.5 {
			continue // prune
		}

		// Find fractional variable to branch on (most fractional)
		branchVar := -1
		maxFrac := 0.0
		for i := 0; i < n; i++ {
			xi := ySol[i] + node.lb[i]
			frac := xi - math.Floor(xi)
			if frac < EPS {
				frac = 0
			} else if frac > 1-EPS {
				frac = 0
			}
			if frac > maxFrac {
				maxFrac = frac
				branchVar = i
			}
		}

		if branchVar == -1 {
			// All integer solution
			val := int64(math.Round(lpVal))
			if val < bestVal {
				bestVal = val
			}
			continue
		}

		xi := ySol[branchVar] + node.lb[branchVar]
		floor := math.Floor(xi)
		ceil := math.Ceil(xi)

		// Down branch: x[branchVar] <= floor
		newLBDown := make([]float64, n)
		newUBDown := make([]float64, n)
		copy(newLBDown, node.lb)
		copy(newUBDown, node.ub)
		if floor < newUBDown[branchVar] {
			newUBDown[branchVar] = floor
		}
		stack = append(stack, Node{lb: newLBDown, ub: newUBDown})

		// Up branch: x[branchVar] >= ceil
		newLBUp := make([]float64, n)
		newUBUp := make([]float64, n)
		copy(newLBUp, node.lb)
		copy(newUBUp, node.ub)
		if ceil > newLBUp[branchVar] {
			newLBUp[branchVar] = ceil
		}
		stack = append(stack, Node{lb: newLBUp, ub: newUBUp})
	}

	if bestVal == math.MaxInt64 {
		return 0, false
	}
	return bestVal, true
}

func solvePart2Machine(m Machine) int64 {
	nCounters := len(m.joltage)
	nButtons := len(m.buttons)

	// A[j][i] = 1 if button i affects counter j
	A := make([][]float64, nCounters)
	for j := 0; j < nCounters; j++ {
		A[j] = make([]float64, nButtons)
	}
	for i, btn := range m.buttons {
		for _, idx := range btn {
			if idx < nCounters {
				A[idx][i] = 1.0
			}
		}
	}

	b := make([]float64, nCounters)
	for j, v := range m.joltage {
		b[j] = float64(v)
	}

	val, ok := solveILP(A, b, nButtons)
	if !ok {
		return -1
	}
	return val
}

func solvePart2(machines []Machine) int64 {
	total := int64(0)
	for _, m := range machines {
		v := solvePart2Machine(m)
		total += v
	}
	return total
}

func main() {
	part := 0
	if len(os.Args) > 1 {
		p, _ := strconv.Atoi(os.Args[1])
		part = p
	}

	machines := readMachines("input.txt")

	if part == 0 || part == 1 {
		fmt.Printf("Part 1: %d\n", solvePart1(machines))
	}
	if part == 0 || part == 2 {
		fmt.Printf("Part 2: %d\n", solvePart2(machines))
	}
}
