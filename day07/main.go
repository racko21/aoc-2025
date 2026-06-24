// Day 7: tachyon beams split downward at '^'.
// Part 1: count splits (beams merge at same column). Part 2: count distinct timelines (counts accumulate).
package main

import (
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/racko21/aoc-2025/utils"
)

func countSplits(grid []string) int {
	rows := len(grid)
	if rows == 0 {
		return 0
	}
	cols := len(grid[0])

	// find S to determine starting column and row
	sr, sc := 0, 0
	for r, row := range grid {
		if c := strings.IndexByte(row, 'S'); c >= 0 {
			sr, sc = r, c
			break
		}
	}

	// active is the set of columns carrying a downward beam
	active := map[int]bool{sc: true}
	total := 0

	for r := sr; r < rows && len(active) > 0; r++ {
		next := map[int]bool{}
		for c := range active {
			if grid[r][c] == '^' {
				total++
				if c-1 >= 0 {
					next[c-1] = true
				}
				if c+1 < cols {
					next[c+1] = true
				}
			} else {
				next[c] = true
			}
		}
		active = next
	}
	return total
}

// countTimelines counts distinct particle timelines exiting the manifold.
// Timelines with the same column at the same row multiply forward independently.
func countTimelines(grid []string) int {
	rows := len(grid)
	if rows == 0 {
		return 0
	}
	cols := len(grid[0])

	sr, sc := 0, 0
	for r, row := range grid {
		if c := strings.IndexByte(row, 'S'); c >= 0 {
			sr, sc = r, c
			break
		}
	}

	active := map[int]int{sc: 1}

	for r := sr; r < rows && len(active) > 0; r++ {
		next := map[int]int{}
		for c, count := range active {
			if grid[r][c] == '^' {
				if c-1 >= 0 {
					next[c-1] += count
				}
				if c+1 < cols {
					next[c+1] += count
				}
			} else {
				next[c] += count
			}
		}
		active = next
	}

	total := 0
	for _, count := range active {
		total += count
	}
	return total
}

func part1(path string) int { return countSplits(utils.ReadLines(path)) }
func part2(path string) int { return countTimelines(utils.ReadLines(path)) }

func main() {
	if len(os.Args) > 1 {
		switch os.Args[1] {
		case "1":
			start := time.Now()
			result := part1("input.txt")
			fmt.Fprintf(os.Stderr, "runtime_ms: %.2f\n", float64(time.Since(start).Microseconds())/1000)
			fmt.Println("Part 1:", result)
			return
		case "2":
			start := time.Now()
			result := part2("input.txt")
			fmt.Fprintf(os.Stderr, "runtime_ms: %.2f\n", float64(time.Since(start).Microseconds())/1000)
			fmt.Println("Part 2:", result)
			return
		}
	}
	fmt.Println("Part 1:", part1("input.txt"))
	fmt.Println("Part 2:", part2("input.txt"))
}
