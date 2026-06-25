//go:build ignore

package main

import (
	"fmt"
)

func main() {
	// Shape 4: ### / #.. / ###
	grid := []string{"###", "#..", "###"}
	sh := parseGrid(grid)
	fmt.Printf("Shape 4: cells=%v rows=%d cols=%d\n", sh.cells, sh.rows, sh.cols)

	orients := allOrientations(sh)
	fmt.Printf("Orientations: %d\n", len(orients))
	for i, o := range orients {
		fmt.Printf("  Orient %d: cells=%v rows=%d cols=%d\n", i, o.cells, o.rows, o.cols)
	}

	// Test placements in 4x4
	W, H := 4, 4
	fmt.Printf("\nPlacements in %dx%d grid:\n", W, H)
	var allP []Placement
	for oi, o := range orients {
		p := computePlacements(o, W, H)
		fmt.Printf("  orient %d rows=%d cols=%d: %d placements\n", oi, o.rows, o.cols, len(p))
		for _, pp := range p {
			fmt.Printf("    cells=%v\n", pp.cells)
		}
		allP = append(allP, p...)
	}
	fmt.Printf("  total: %d placements\n", len(allP))

	// Check which placements cover cell 0
	fmt.Printf("\nPlacements covering cell 0:\n")
	for i, p := range allP {
		for _, cell := range p.cells {
			if cell == 0 {
				fmt.Printf("  placement %d: cells=%v\n", i, p.cells)
				break
			}
		}
	}

	fmt.Printf("\nTotal cells in shape: %d\n", len(sh.cells))
	fmt.Printf("Grid size: %d\n", W*H)

	// Try manually: can we place 2 of shape 4 in 4x4?
	// AAA.   cells for A: 0,1,2,4,8 (if shape is ### / #.. / ###)
	// ABAB   and B: ...
	// ABAB
	// .BBB
	// Let me figure out which placements those are
	// grid is 4 wide, so:
	// (0,0)=0, (0,1)=1, (0,2)=2, (0,3)=3
	// (1,0)=4, (1,1)=5, (1,2)=6, (1,3)=7
	// (2,0)=8, (2,1)=9, (2,2)=10,(2,3)=11
	// (3,0)=12,(3,1)=13,(3,2)=14,(3,3)=15

	// AAA.  → A at 0,1,2
	// ABAB  → A at 4, B at 5, A at 6? No, "ABAB" = A,B,A,B
	// Wait that can't be right for a solid shape
	// Let me re-read the example
	// AAA.
	// ABAB  → row 1: A,B,A,B?
	// That means A has cells: (0,0),(0,1),(0,2),(1,0),(1,2),(2,0),(2,2) = 7 cells! 
	// That's shape ### / #.# / ### which is a ROTATION of shape 4!
	// And B has cells: (1,1),(1,3),(2,1),(2,3),(3,1),(3,2),(3,3) = 7 cells
	// B shape = .# / .# / ### - also a rotation!

	fmt.Printf("\nA cells: 0,1,2,4,6,8,10\n")
	fmt.Printf("B cells: 5,7,9,11,13,14,15\n")
	// Check if both are valid placements
	for i, p := range allP {
		match := true
		target := []int{0, 1, 2, 4, 6, 8, 10}
		if len(p.cells) != len(target) {
			continue
		}
		for _, c := range p.cells {
			found := false
			for _, t := range target {
				if c == t {
					found = true
					break
				}
			}
			if !found {
				match = false
				break
			}
		}
		if match {
			fmt.Printf("A matches placement %d: %v\n", i, p.cells)
		}
	}
}
