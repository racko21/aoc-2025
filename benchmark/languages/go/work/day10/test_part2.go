//go:build ignore

package main

import (
	"fmt"
	"math"
)

const EPS2 = 1e-9
const INF2 = 1e18

func simplex2(A [][]float64, b []float64, c []float64) (float64, []float64, bool) {
	m := len(A)
	n := len(c)

	for i := 0; i < m; i++ {
		if b[i] < -EPS2 {
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
	for j := 0; j < total+1; j++ {
		tab[m][j] = 0
	}
	for i := 0; i < m; i++ {
		for j := 0; j < total+1; j++ {
			tab[m][j] -= tab[i][j]
		}
	}

	basis := make([]int, m)
	for i := 0; i < m; i++ {
		basis[i] = n + i
	}

	runSimplex2(tab, basis, m, total)

	phase1Val := -tab[m][total]
	fmt.Printf("Phase 1 val: %f\n", phase1Val)
	if phase1Val > EPS2 {
		return 0, nil, false
	}

	for i := 0; i < m; i++ {
		if basis[i] >= n {
			for j := 0; j < n; j++ {
				if math.Abs(tab[i][j]) > EPS2 {
					pivot2(tab, basis, i, j, m, total)
					break
				}
			}
		}
	}

	for j := 0; j < total+1; j++ {
		tab[m][j] = 0
	}
	for j := 0; j < n; j++ {
		tab[m][j] = c[j]
	}
	for i := 0; i < m; i++ {
		bv := basis[i]
		if bv < n {
			coef := tab[m][bv]
			if math.Abs(coef) > EPS2 {
				for j := 0; j < total+1; j++ {
					tab[m][j] -= coef * tab[i][j]
				}
			}
		}
	}

	runSimplex2(tab, basis, m, total)

	optVal := -tab[m][total]
	x := make([]float64, n)
	for i := 0; i < m; i++ {
		if basis[i] < n {
			x[basis[i]] = tab[i][total]
		}
	}
	return optVal, x, true
}

func runSimplex2(tab [][]float64, basis []int, m, total int) {
	for iter := 0; iter < 10000; iter++ {
		enterCol := -1
		minVal := -EPS2
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
		minRatio := INF2
		for i := 0; i < m; i++ {
			if tab[i][enterCol] > EPS2 {
				ratio := tab[i][total] / tab[i][enterCol]
				if ratio < minRatio-EPS2 {
					minRatio = ratio
					leaveRow = i
				} else if math.Abs(ratio-minRatio) < EPS2 && leaveRow >= 0 {
					if basis[i] < basis[leaveRow] {
						leaveRow = i
					}
				}
			}
		}
		if leaveRow == -1 {
			break
		}
		pivot2(tab, basis, leaveRow, enterCol, m, total)
	}
}

func pivot2(tab [][]float64, basis []int, row, col, m, total int) {
	pivVal := tab[row][col]
	for j := 0; j <= total; j++ {
		tab[row][j] /= pivVal
	}
	for i := 0; i <= m; i++ {
		if i == row {
			continue
		}
		factor := tab[i][col]
		if math.Abs(factor) < EPS2 {
			continue
		}
		for j := 0; j <= total; j++ {
			tab[i][j] -= factor * tab[row][j]
		}
	}
	basis[row] = col
}

func main() {
	// Machine 1: [.##.] (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}
	// Buttons: 0=(3), 1=(1,3), 2=(2), 3=(2,3), 4=(0,2), 5=(0,1)
	A := [][]float64{
		{0, 0, 0, 0, 1, 1}, // counter 0 = 3
		{0, 1, 0, 0, 0, 1}, // counter 1 = 5
		{0, 0, 1, 1, 1, 0}, // counter 2 = 4
		{1, 1, 0, 1, 0, 0}, // counter 3 = 7
	}
	b := []float64{3, 5, 4, 7}
	c := []float64{1, 1, 1, 1, 1, 1}
	
	val, x, ok := simplex2(A, b, c)
	fmt.Printf("Feasible: %v, opt val: %f\n", ok, val)
	fmt.Printf("x: %v\n", x)
}
