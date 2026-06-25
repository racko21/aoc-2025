package main

import (
	"fmt"
)

func main2() {
	// Test machine 1: [.##.] (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}
	// Buttons: (3), (1,3), (2), (2,3), (0,2), (0,1)
	// Joltage: 3,5,4,7
	// A[counter][button]
	// counter 0: buttons with 0: (0,2)=btn4, (0,1)=btn5
	// counter 1: buttons with 1: (1,3)=btn1, (0,1)=btn5
	// counter 2: buttons with 2: (2)=btn2, (2,3)=btn3, (0,2)=btn4
	// counter 3: buttons with 3: (3)=btn0, (1,3)=btn1, (2,3)=btn3
	
	// So A:
	// [0, 0, 0, 0, 1, 1]  = 3
	// [0, 1, 0, 0, 0, 1]  = 5
	// [0, 0, 1, 1, 1, 0]  = 4
	// [1, 1, 0, 1, 0, 0]  = 7
	
	// The solution says: press (3) once, (1,3) three times, (2,3) three times, (0,2) once, (0,1) twice
	// x = [1, 3, 0, 3, 1, 2]
	// sum = 10 ✓
	
	// Let's check my matrix
	fmt.Println("Machine 1 matrix check:")
	x := []float64{1, 3, 0, 3, 1, 2}
	b := []float64{3, 5, 4, 7}
	A := [][]float64{
		{0, 0, 0, 0, 1, 1},
		{0, 1, 0, 0, 0, 1},
		{0, 0, 1, 1, 1, 0},
		{1, 1, 0, 1, 0, 0},
	}
	for j := 0; j < 4; j++ {
		sum := 0.0
		for i := 0; i < 6; i++ {
			sum += A[j][i] * x[i]
		}
		fmt.Printf("  counter %d: got %.0f, want %.0f\n", j, sum, b[j])
	}
}

func init() {
	main2()
}
