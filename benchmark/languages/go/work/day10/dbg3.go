//go:build ignore

package main

import (
	"fmt"
	"math"
)

const EPS = 1e-9

func twoPhase(Aorig [][]float64, borig []float64, corig []float64) (float64, []float64, bool) {
	m := len(Aorig)
	n := len(corig)
	if m == 0 {
		return 0, make([]float64, n), true
	}

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

	N := n + m
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

	// Phase 1 objective: minimize sum of artificials
	phase1obj := make([]float64, N+1)
	for i := 0; i < m; i++ {
		phase1obj[n+i] = 1.0
	}
	// Eliminate basic variables from obj
	for i := 0; i < m; i++ {
		coef := phase1obj[n+i]
		for j := 0; j <= N; j++ {
			phase1obj[j] -= coef * tab[i][j]
		}
	}

	fmt.Printf("Phase 1 obj row: %v\n", phase1obj)

	simplexIterate(tab, basis, phase1obj, m, N)
	
	fmt.Printf("After phase1: basis=%v, obj RHS=%f\n", basis, phase1obj[N])
	fmt.Printf("Phase 1 z = %f\n", -phase1obj[N])

	z1 := -phase1obj[N]
	if z1 > EPS {
		return 0, nil, false
	}

	for i := 0; i < m; i++ {
		if basis[i] >= n {
			for j := 0; j < n; j++ {
				if math.Abs(tab[i][j]) > EPS {
					simplexPivot(tab, basis, phase1obj, i, j, m, N)
					break
				}
			}
		}
	}

	fmt.Printf("After artificial removal: basis=%v\n", basis)

	// Phase 2
	phase2obj := make([]float64, N+1)
	for j := 0; j < n; j++ {
		phase2obj[j] = corig[j]
	}
	for i := 0; i < m; i++ {
		if basis[i] < n {
			coef := phase2obj[basis[i]]
			if math.Abs(coef) > EPS {
				for j := 0; j <= N; j++ {
					phase2obj[j] -= coef * tab[i][j]
				}
			}
		}
	}

	fmt.Printf("Phase 2 obj row before iterate: %v\n", phase2obj)

	simplexIterate(tab, basis, phase2obj, m, N)

	fmt.Printf("After phase2: basis=%v, obj RHS=%f\n", basis, phase2obj[N])
	fmt.Printf("Phase 2 z = %f\n", -phase2obj[N])

	optVal := -phase2obj[N]
	x := make([]float64, n)
	for i := 0; i < m; i++ {
		if basis[i] < n {
			x[basis[i]] = tab[i][N]
		}
	}
	return optVal, x, true
}

func simplexIterate(tab [][]float64, basis []int, obj []float64, m, N int) {
	for iter := 0; iter < 200000; iter++ {
		enterCol := -1
		minVal := -EPS
		for j := 0; j < N; j++ {
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
			break
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

func main() {
	// Simple test: minimize x0+x1 s.t. x0+x1 = 5, x0,x1 >= 0
	fmt.Println("=== Test 1: min x0+x1 s.t. x0+x1=5 ===")
	A1 := [][]float64{{1, 1}}
	b1 := []float64{5}
	c1 := []float64{1, 1}
	v1, x1, ok1 := twoPhase(A1, b1, c1)
	fmt.Printf("ok=%v val=%f x=%v\n\n", ok1, v1, x1)

	// Test from machine 1 basic (without upper bounds)
	fmt.Println("=== Test 2: machine 1 basic LP ===")
	A2 := [][]float64{
		{0, 0, 0, 0, 1, 1},
		{0, 1, 0, 0, 0, 1},
		{0, 0, 1, 1, 1, 0},
		{1, 1, 0, 1, 0, 0},
	}
	b2 := []float64{3, 5, 4, 7}
	c2 := []float64{1, 1, 1, 1, 1, 1}
	v2, x2, ok2 := twoPhase(A2, b2, c2)
	fmt.Printf("ok=%v val=%f x=%v\n\n", ok2, v2, x2)
}
