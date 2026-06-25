package main

import (
	"bufio"
	"fmt"
	"math/bits"
	"os"
	"strconv"
	"strings"
)

type machine struct {
	nl    int
	masks []int // for part1: bitmask of lights each button toggles
	btns  [][]int
	light []int // for part1: target bits
	jolt  []int
}

func parse(line string) machine {
	var m machine
	lb := strings.Index(line, "[")
	rb := strings.Index(line, "]")
	lights := line[lb+1 : rb]
	m.nl = len(lights)
	m.light = make([]int, m.nl)
	for i, c := range lights {
		if c == '#' {
			m.light[i] = 1
		}
	}
	// buttons
	rest := line[rb+1:]
	for {
		lp := strings.Index(rest, "(")
		if lp < 0 {
			break
		}
		rp := strings.Index(rest, ")")
		inner := rest[lp+1 : rp]
		parts := strings.Split(inner, ",")
		var b []int
		mask := 0
		for _, p := range parts {
			v, _ := strconv.Atoi(strings.TrimSpace(p))
			b = append(b, v)
			mask |= 1 << v
		}
		m.btns = append(m.btns, b)
		m.masks = append(m.masks, mask)
		rest = rest[rp+1:]
	}
	// jolt
	lc := strings.Index(line, "{")
	rc := strings.Index(line, "}")
	jparts := strings.Split(line[lc+1:rc], ",")
	for _, p := range jparts {
		v, _ := strconv.Atoi(strings.TrimSpace(p))
		m.jolt = append(m.jolt, v)
	}
	return m
}

// Part 1: minimal number of buttons (each pressed 0/1) to XOR to target.
func part1machine(m machine) int {
	target := 0
	for i, v := range m.light {
		if v == 1 {
			target |= 1 << i
		}
	}
	nb := len(m.masks)
	best := -1
	for s := 0; s < (1 << nb); s++ {
		x := 0
		for i := 0; i < nb; i++ {
			if s&(1<<i) != 0 {
				x ^= m.masks[i]
			}
		}
		if x == target {
			c := bits.OnesCount(uint(s))
			if best < 0 || c < best {
				best = c
			}
		}
	}
	return best
}


func main() {
	part := 0
	if len(os.Args) > 1 {
		part, _ = strconv.Atoi(os.Args[1])
	}
	f, _ := os.Open("input.txt")
	defer f.Close()
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 1024*1024), 1024*1024)
	var machines []machine
	for sc.Scan() {
		line := strings.TrimSpace(sc.Text())
		if line == "" {
			continue
		}
		machines = append(machines, parse(line))
	}
	if part == 0 || part == 1 {
		sum := 0
		for _, m := range machines {
			sum += part1machine(m)
		}
		fmt.Printf("Part 1: %d\n", sum)
	}
	if part == 0 || part == 2 {
		sum := 0
		for _, m := range machines {
			sum += part2machine(m)
		}
		fmt.Printf("Part 2: %d\n", sum)
	}
}

// Part 2 via Gaussian elimination + bounded search over free variables.
// System: A x = jolt, x >= 0 integer. A is nj x nb (0/1). Minimize sum(x).

func part2machine(m machine) int {
	nb := len(m.btns)
	nj := len(m.jolt)
	// Build augmented matrix as float64 (entries 0/1, integer rhs).
	// rows = nj, cols = nb, plus rhs.
	mat := make([][]float64, nj)
	for i := range mat {
		mat[i] = make([]float64, nb+1)
	}
	for bi, b := range m.btns {
		for _, j := range b {
			if j < nj {
				mat[j][bi] = 1
			}
		}
	}
	for j := 0; j < nj; j++ {
		mat[j][nb] = float64(m.jolt[j])
	}
	// Gaussian elimination to RREF
	pivotCol := make([]int, 0)
	rowOfPivot := make([]int, 0)
	r := 0
	const eps = 1e-9
	for c := 0; c < nb && r < nj; c++ {
		// find pivot
		piv := -1
		best := eps
		for i := r; i < nj; i++ {
			if abs(mat[i][c]) > best {
				best = abs(mat[i][c])
				piv = i
			}
		}
		if piv < 0 {
			continue
		}
		mat[r], mat[piv] = mat[piv], mat[r]
		pv := mat[r][c]
		for k := 0; k <= nb; k++ {
			mat[r][k] /= pv
		}
		for i := 0; i < nj; i++ {
			if i != r && abs(mat[i][c]) > eps {
				f := mat[i][c]
				for k := 0; k <= nb; k++ {
					mat[i][k] -= f * mat[r][k]
				}
			}
		}
		pivotCol = append(pivotCol, c)
		rowOfPivot = append(rowOfPivot, r)
		r++
	}
	// check consistency: rows beyond r with nonzero rhs
	for i := r; i < nj; i++ {
		if abs(mat[i][nb]) > 1e-6 {
			return -1 // infeasible (shouldn't happen)
		}
	}
	isPivot := make([]bool, nb)
	for _, c := range pivotCol {
		isPivot[c] = true
	}
	var freeVars []int
	for c := 0; c < nb; c++ {
		if !isPivot[c] {
			freeVars = append(freeVars, c)
		}
	}
	// dependent var p (pivotCol[k]) = mat[row][nb] - sum_{free f} mat[row][f]*x[f]
	// We need integer nonneg solution minimizing total sum.
	nfree := len(freeVars)
	// upper bounds for each free var
	ub := make([]int, nfree)
	for i, fv := range freeVars {
		// min target over lights this button affects
		mn := 1 << 60
		for _, j := range m.btns[fv] {
			if j < nj && m.jolt[j] < mn {
				mn = m.jolt[j]
			}
		}
		if mn == 1<<60 {
			mn = 0
		}
		ub[i] = mn
	}
	x := make([]int, nb)
	best := -1
	fvals := make([]int, nfree)

	var rec func(idx int)
	rec = func(idx int) {
		if idx == nfree {
			// compute dependent vars
			total := 0
			for i := range freeVars {
				x[freeVars[i]] = fvals[i]
				total += fvals[i]
			}
			for k, pc := range pivotCol {
				row := rowOfPivot[k]
				val := mat[row][nb]
				for i, fv := range freeVars {
					val -= mat[row][fv] * float64(fvals[i])
				}
				// must be nonneg integer
				rv := int(val + 0.5)
				if val < -1e-6 {
					return
				}
				if abs(val-float64(rv)) > 1e-6 {
					return
				}
				if rv < 0 {
					return
				}
				x[pc] = rv
				total += rv
			}
			if best < 0 || total < best {
				best = total
			}
			return
		}
		for v := 0; v <= ub[idx]; v++ {
			fvals[idx] = v
			rec(idx + 1)
		}
	}
	rec(0)
	return best
}

func abs(x float64) float64 {
	if x < 0 {
		return -x
	}
	return x
}

