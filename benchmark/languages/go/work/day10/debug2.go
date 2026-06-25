//go:build ignore

package main

import (
	"fmt"
	"math"
)

const EPS = 1e-9
const BigM = 1e8

func simplexSolve(Aorig [][]float64, borig []float64, corig []float64) (float64, []float64, bool) {
	m := len(Aorig)
	n := len(corig)

	A := make([][]float64, m)
	b := make([]float64, m)
	c := make([]float64, n)
	for i := 0; i < m; i++ {
		A[i] = make([]float64, n)
		copy(A[i], Aorig[i])
		b[i] = borig[i]
	}
	copy(c, corig)

	for i := 0; i < m; i++ {
		if b[i] < 0 {
			b[i] = -b[i]
			for j := 0; j < n; j++ {
				A[i][j] = -A[i][j]
			}
		}
	}

	total := n + m
	tab := make([][]float64, m+1)
	for i := 0; i <= m; i++ {
		tab[i] = make([]float64, total+1)
	}

	for i := 0; i < m; i++ {
		for j := 0; j < n; j++ {
			tab[i][j] = A[i][j]
		}
		tab[i][n+i] = 1.0
		tab[i][total] = b[i]
	}

	for j := 0; j < n; j++ {
		tab[m][j] = c[j]
	}
	for i := 0; i < m; i++ {
		tab[m][n+i] = BigM
	}

	basis := make([]int, m)
	for i := 0; i < m; i++ {
		basis[i] = n + i
	}
	for i := 0; i < m; i++ {
		coef := tab[m][n+i]
		for j := 0; j <= total; j++ {
			tab[m][j] -= coef * tab[i][j]
		}
	}

	fmt.Println("Initial objective row:", tab[m])

	for iter := 0; iter < 100000; iter++ {
		enterCol := -1
		minVal := -EPS
		for j := 0; j < total; j++ {
			if tab[m][j] < minVal {
				minVal = tab[m][j]
				enterCol = j
			}
		}
		if enterCol == -1 {
			break
		}

		leaveRow := -1
		minRatio := 1e18
		for i := 0; i < m; i++ {
			if tab[i][enterCol] > EPS {
				ratio := tab[i][total] / tab[i][enterCol]
				if ratio < minRatio-EPS || (math.Abs(ratio-minRatio) < EPS && leaveRow >= 0 && basis[i] < basis[leaveRow]) {
					minRatio = ratio
					leaveRow = i
				}
			}
		}
		if leaveRow == -1 {
			return 0, nil, false
		}

		pivVal := tab[leaveRow][enterCol]
		for j := 0; j <= total; j++ {
			tab[leaveRow][j] /= pivVal
		}
		for i := 0; i <= m; i++ {
			if i == leaveRow {
				continue
			}
			factor := tab[i][enterCol]
			if math.Abs(factor) < EPS {
				continue
			}
			for j := 0; j <= total; j++ {
				tab[i][j] -= factor * tab[leaveRow][j]
			}
		}
		basis[leaveRow] = enterCol
	}

	fmt.Println("Final basis:", basis)
	fmt.Println("Final objective:", tab[m][total])
	for i := 0; i < m; i++ {
		fmt.Printf("  Row %d basis=%d val=%f\n", i, basis[i], tab[i][total])
	}

	for i := 0; i < m; i++ {
		if basis[i] >= n && tab[i][total] > EPS {
			return 0, nil, false
		}
	}

	optVal := tab[m][total]
	x := make([]float64, n)
	for i := 0; i < m; i++ {
		if basis[i] < n {
			x[basis[i]] = tab[i][total]
		}
	}
	return optVal, x, true
}

func main() {
	// Machine 1: [.##.] (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}
	// n=6 original vars, m=4 constraints
	// After adding upper bound slacks: n=6+6=12 vars, m=4+6=10 constraints
	
	// But first let's test the basic LP without bounds
	// Variables: x0..x5 (button presses), artificials x6..x9
	A := [][]float64{
		{0, 0, 0, 0, 1, 1}, // counter 0 = 3
		{0, 1, 0, 0, 0, 1}, // counter 1 = 5
		{0, 0, 1, 1, 1, 0}, // counter 2 = 4
		{1, 1, 0, 1, 0, 0}, // counter 3 = 7
	}
	b := []float64{3, 5, 4, 7}
	c := []float64{1, 1, 1, 1, 1, 1}
	
	fmt.Println("=== Basic LP (no upper bounds) ===")
	val, x, ok := simplexSolve(A, b, c)
	fmt.Printf("Feasible: %v, opt val: %f, x: %v\n", ok, val, x)
	
	// Now with upper bounds (n=6, upper bound=1e9)
	// Extended: 12 vars, 10 constraints
	n := 6
	m := 4
	AExt := make([][]float64, m+n)
	bExt := make([]float64, m+n)
	for j := 0; j < m; j++ {
		AExt[j] = make([]float64, 2*n)
		for i := 0; i < n; i++ {
			AExt[j][i] = A[j][i]
		}
		bExt[j] = b[j]
	}
	for i := 0; i < n; i++ {
		AExt[m+i] = make([]float64, 2*n)
		AExt[m+i][i] = 1.0
		AExt[m+i][n+i] = 1.0
		bExt[m+i] = 1e9
	}
	cExt := make([]float64, 2*n)
	for i := 0; i < n; i++ {
		cExt[i] = 1.0
	}
	
	fmt.Println("\n=== Extended LP (with upper bounds) ===")
	val2, x2, ok2 := simplexSolve(AExt, bExt, cExt)
	fmt.Printf("Feasible: %v, opt val: %f, x: %v\n", ok2, val2, x2[:n])
}
