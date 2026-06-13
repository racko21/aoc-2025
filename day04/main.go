// Day 4: count '@' cells with fewer than 4 '@' neighbors (8-directional).
package main

import (
	"fmt"

	"github.com/racko21/aoc-2025/utils"
)

func parseGrid(path string) []string {
	return utils.ReadLines(path)
}

func neighbors(grid []string, r, c int) int {
	rows, cols := len(grid), len(grid[0])
	count := 0
	for dr := -1; dr <= 1; dr++ {
		for dc := -1; dc <= 1; dc++ {
			if dr == 0 && dc == 0 {
				continue
			}
			nr, nc := r+dr, c+dc
			if nr >= 0 && nr < rows && nc >= 0 && nc < cols && grid[nr][nc] == '@' {
				count++
			}
		}
	}
	return count
}

func part1(grid []string) int {
	count := 0
	for r, row := range grid {
		for c, ch := range row {
			if ch == '@' && neighbors(grid, r, c) < 4 {
				count++
			}
		}
	}
	return count
}

// toBytes converts []string grid to mutable [][]byte.
func toBytes(grid []string) [][]byte {
	g := make([][]byte, len(grid))
	for i, row := range grid {
		g[i] = []byte(row)
	}
	return g
}

// neighborsB counts '@' neighbors in a [][]byte grid.
func neighborsB(grid [][]byte, r, c int) int {
	rows, cols := len(grid), len(grid[0])
	count := 0
	for dr := -1; dr <= 1; dr++ {
		for dc := -1; dc <= 1; dc++ {
			if dr == 0 && dc == 0 {
				continue
			}
			nr, nc := r+dr, c+dc
			if nr >= 0 && nr < rows && nc >= 0 && nc < cols && grid[nr][nc] == '@' {
				count++
			}
		}
	}
	return count
}

func part2(grid []string) int {
	g := toBytes(grid)
	total := 0
	for {
		// collect all accessible rolls in this pass
		var toRemove [][2]int
		for r, row := range g {
			for c, ch := range row {
				if ch == '@' && neighborsB(g, r, c) < 4 {
					toRemove = append(toRemove, [2]int{r, c})
				}
			}
		}
		if len(toRemove) == 0 {
			break
		}
		for _, pos := range toRemove {
			g[pos[0]][pos[1]] = '.'
		}
		total += len(toRemove)
	}
	return total
}

func main() {
	grid := parseGrid("input.txt")
	fmt.Println("Part 1:", part1(grid))
	fmt.Println("Part 2:", part2(grid))
}
