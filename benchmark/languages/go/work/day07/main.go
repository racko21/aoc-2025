package main

import (
	"bufio"
	"fmt"
	"os"
)

func readGrid(filename string) []string {
	f, err := os.Open(filename)
	if err != nil {
		panic(err)
	}
	defer f.Close()
	var grid []string
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := scanner.Text()
		if line != "" {
			grid = append(grid, line)
		}
	}
	return grid
}

// For Part 1: BFS/simulation of beams traveling downward.
// Each beam is identified by (col, row) where it starts traveling downward.
// When it hits a ^, it splits: two new beams start at (col-1, splitterRow) and (col+1, splitterRow)
// traveling downward.
// Count total number of unique splitter hits (activations).

func solve(grid []string) (int, int) {
	rows := len(grid)
	cols := len(grid[0])

	// Find S position (starting column, row 0)
	startCol := -1
	for c := 0; c < cols; c++ {
		if grid[0][c] == 'S' {
			startCol = c
			break
		}
	}

	// We simulate beams. A beam travels downward from a given column starting at a given row.
	// State: (col, startRow) - beam traveling down column col starting from row startRow.
	// We use BFS and track which splitters have been activated.

	// For Part 1: count unique splitter activations.
	// For Part 2: count number of timelines = number of leaf paths.

	// Think of it as a DAG:
	// - Root: beam at (startCol, row 1) going down (after S at row 0)
	//   Actually beam starts at row 0 from S, going down.
	// 
	// For each beam (col, startRow going down):
	//   Scan rows from startRow downward in column col.
	//   If we hit a '^' at (col, r):
	//     - This splitter is activated (count it for part 1 if not yet counted)
	//     - Spawn two beams: (col-1, r) and (col+1, r) going down
	//     - (if col-1 or col+1 out of bounds, that beam just exits)
	//   If we exit the grid without hitting a splitter: this is a leaf (for part 2, +1 timeline)

	// For Part 2: number of timelines = number of paths that reach a leaf (exit without splitting further).
	// We compute this recursively with memoization.
	// timelines(col, startRow) = 
	//   if next splitter is at row r:
	//     timelines(col-1, r) + timelines(col+1, r)   [but handle out-of-bounds: OOB = 1 timeline]
	//   else (no splitter found):
	//     1

	// But wait: if two beams arrive at the same splitter from the same direction,
	// in Part 1 the splitter counts once (beams merge).
	// In Part 2, beams are independent timelines so they don't merge.

	// Actually re-reading Part 2: "A tachyon particle takes both the left and right path"
	// and we count distinct timelines. If two paths converge to the same splitter,
	// they still count as separate timelines past that point.
	// So for Part 2, timelines(col, startRow) with memoization where the result
	// is the count of leaf exits from that beam state.

	// For Part 1: splitters activated = unique (col, row) splitter positions that are hit.

	// Let's find all splitter positions: grid[r][c] == '^'
	// For a beam at column col starting at row startRow going down:
	//   find smallest r >= startRow such that grid[r][col] == '^'
	//   (Also S doesn't block beams, it's just the starting marker)

	// Memoization for Part 2:
	// memo[col][startRow] = number of leaf timelines from beam (col, startRow going down)
	memo := make([][]int, cols)
	computed := make([][]bool, cols)
	for c := 0; c < cols; c++ {
		memo[c] = make([]int, rows)
		computed[c] = make([]bool, rows)
	}

	// Set of activated splitters for Part 1
	activated := make(map[[2]int]bool)

	// Find next splitter in column col at or below row startRow
	// Returns row of splitter, or -1 if none
	nextSplitter := func(col, startRow int) int {
		for r := startRow; r < rows; r++ {
			if grid[r][col] == '^' {
				return r
			}
		}
		return -1
	}

	// We need to do two things:
	// 1. Simulate all reachable beam states (BFS) to find activated splitters (Part 1)
	// 2. Count timelines (Part 2) via memoized recursion

	// Actually for Part 1, we also need to know which splitters are reachable.
	// Let's use BFS for Part 1, then memoized recursion for Part 2 restricted to reachable splitters.

	// Wait - for Part 2, the timelines function doesn't care about "reachable" from a global perspective;
	// it's just the tree structure. Every beam that reaches a splitter spawns 2 children.
	// If col-1 < 0 or col+1 >= cols, that child just exits (1 timeline).

	// BFS for reachability (Part 1):
	type BeamState struct{ col, row int }
	visited := make(map[BeamState]bool)
	queue := []BeamState{{startCol, 0}}
	visited[BeamState{startCol, 0}] = true

	for len(queue) > 0 {
		b := queue[0]
		queue = queue[1:]

		r := nextSplitter(b.col, b.row)
		if r == -1 {
			// beam exits, no split
			continue
		}
		// Splitter at (b.col, r) is activated
		activated[[2]int{b.col, r}] = true

		// Spawn children
		children := []BeamState{}
		if b.col-1 >= 0 {
			children = append(children, BeamState{b.col - 1, r})
		}
		if b.col+1 < cols {
			children = append(children, BeamState{b.col + 1, r})
		}
		for _, child := range children {
			if !visited[child] {
				visited[child] = true
				queue = append(queue, child)
			}
		}
	}

	part1 := len(activated)

	// Part 2: count timelines via memoized recursion
	// timelines(col, startRow) = number of leaf exits
	var countTimelines func(col, startRow int) int
	countTimelines = func(col, startRow int) int {
		if col < 0 || col >= cols {
			return 1 // exited left/right
		}
		if computed[col][startRow] {
			return memo[col][startRow]
		}
		computed[col][startRow] = true

		r := nextSplitter(col, startRow)
		if r == -1 {
			// no splitter found, beam exits bottom
			memo[col][startRow] = 1
			return 1
		}
		// split at row r: children are (col-1, r) and (col+1, r)
		left := 1
		right := 1
		if col-1 >= 0 {
			left = countTimelines(col-1, r)
		}
		if col+1 < cols {
			right = countTimelines(col+1, r)
		}
		result := left + right
		memo[col][startRow] = result
		return result
	}

	part2 := countTimelines(startCol, 0)

	return part1, part2
}

func main() {
	grid := readGrid("input.txt")

	part := ""
	if len(os.Args) > 1 {
		part = os.Args[1]
	}

	p1, p2 := solve(grid)

	switch part {
	case "1":
		fmt.Printf("Part 1: %d\n", p1)
	case "2":
		fmt.Printf("Part 2: %d\n", p2)
	default:
		fmt.Printf("Part 1: %d\n", p1)
		fmt.Printf("Part 2: %d\n", p2)
	}
}
