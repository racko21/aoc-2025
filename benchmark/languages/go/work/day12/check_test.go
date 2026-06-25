package main

import (
	"fmt"
	"testing"
)

func TestShape4(t *testing.T) {
	// Shape 4: ### / #.. / ###
	grid := []string{"###", "#..", "###"}
	sh := parseGrid(grid)
	fmt.Printf("Shape 4: cells=%v rows=%d cols=%d\n", sh.cells, sh.rows, sh.cols)

	orients := allOrientations(sh)
	fmt.Printf("Orientations: %d\n", len(orients))
	for i, o := range orients {
		fmt.Printf("  Orient %d: cells=%v rows=%d cols=%d\n", i, o.cells, o.rows, o.cols)
	}

	W, H := 4, 4
	var allP []Placement
	for oi, o := range orients {
		p := computePlacements(o, W, H)
		fmt.Printf("  orient %d rows=%d cols=%d: %d placements\n", oi, o.rows, o.cols, len(p))
		allP = append(allP, p...)
	}
	fmt.Printf("total: %d placements\n", len(allP))

	// The example solution places A at cells 0,1,2,4,6,8,10
	// which would be orientation ### / #.# / ### (7 cells in 3x3)
	// After rotate: columns 0,1,2 of rows 0,2 and row 0,2 of col 0,1,2
	// Actually shape ### / #.# / ### has cells: (0,0)(0,1)(0,2)(1,0)(1,2)(2,0)(2,1)(2,2) = 8 cells!

	// Wait, let me re-read: "AAA. / ABAB / ABAB / .BBB"
	// Row 0: A A A .
	// Row 1: A B A B
	// Row 2: A B A B  <- wait same as row 1?
	// That can't be right. Let me read more carefully.
	// "AAA. / ABAB / ABAB / .BBB"
	// Row 0: A A A .   => cells 0,1,2
	// Row 1: A B A B   => A at 4,6; B at 5,7
	// Wait but row1 in example is "ABAB" not "AABA"
	// Hmm: A=0,1,2,4,6 and B=5,7,...
	// But A would need cells: 0,1,2,4,6,?
	// Row 2: A B A B = 8,10,? but that gives A=0,1,2,4,6,8,10 = 7 cells ✓
	// Row 3: . B B B = 13,14,15 => B has 5,7,9,11,13,14,15 = 7 cells ✓

	// A = {0,1,2,4,6,8,10} in 4-wide grid:
	// (0,0)(0,1)(0,2)(1,0)(1,2)(2,0)(2,2) -> bounding box 3x3
	// That shape is: ### / #.# / #.# (since (1,1) is missing, (2,1) is missing)
	// Shape is: col 0 full (3 cells), top row full (3 cells), but (1,2) and (2,2) are there
	// Let me draw: 
	// (0,0)(0,1)(0,2) -> # # #
	// (1,0)    (1,2)  -> # . #
	// (2,0)    (2,2)  -> # . #
	// That's 7 cells. This IS a rotation of shape ### / #.. / ###
	// Original: (0,0)(0,1)(0,2)(1,0)(2,0)(2,1)(2,2)
	// Rotate 90 CCW: (r,c)->(c,-r) then norm
	// (0,0)->(0,0), (0,1)->(1,0), (0,2)->(2,0)
	// (1,0)->(0,-1)->(0,2) with norm offset +1 in col... wait
	// Let me just check if {0,1,2,4,6,8,10} appears in allP

	target := []int{0, 1, 2, 4, 6, 8, 10}
	fmt.Printf("\nLooking for A placement %v:\n", target)
	for i, p := range allP {
		if setEqual(p.cells, target) {
			fmt.Printf("  FOUND at placement %d: %v\n", i, p.cells)
		}
	}

	// Also check B = {5,7,9,11,13,14,15}
	targetB := []int{5, 7, 9, 11, 13, 14, 15}
	fmt.Printf("Looking for B placement %v:\n", targetB)
	for i, p := range allP {
		if setEqual(p.cells, targetB) {
			fmt.Printf("  FOUND at placement %d: %v\n", i, p.cells)
		}
	}
}

func setEqual(a, b []int) bool {
	if len(a) != len(b) {
		return false
	}
	m := make(map[int]bool)
	for _, x := range a {
		m[x] = true
	}
	for _, x := range b {
		if !m[x] {
			return false
		}
	}
	return true
}
