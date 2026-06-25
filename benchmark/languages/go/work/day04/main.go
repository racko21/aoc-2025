package main

import (
	"bufio"
	"fmt"
	"os"
)

func readGrid(filename string) [][]byte {
	f, err := os.Open(filename)
	if err != nil {
		panic(err)
	}
	defer f.Close()

	var grid [][]byte
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := scanner.Text()
		if line == "" {
			continue
		}
		grid = append(grid, []byte(line))
	}
	return grid
}

// countNeighbors counts how many of the 8 adjacent cells are '@'
func countNeighbors(grid [][]byte, r, c int) int {
	rows := len(grid)
	cols := len(grid[0])
	count := 0
	for dr := -1; dr <= 1; dr++ {
		for dc := -1; dc <= 1; dc++ {
			if dr == 0 && dc == 0 {
				continue
			}
			nr, nc := r+dr, c+dc
			if nr >= 0 && nr < rows && nc >= 0 && nc < cols {
				if grid[nr][nc] == '@' {
					count++
				}
			}
		}
	}
	return count
}

// accessible returns true if a roll at (r,c) has fewer than 4 '@' neighbors
func accessible(grid [][]byte, r, c int) bool {
	return grid[r][c] == '@' && countNeighbors(grid, r, c) < 4
}

func part1(grid [][]byte) int {
	count := 0
	for r := range grid {
		for c := range grid[r] {
			if accessible(grid, r, c) {
				count++
			}
		}
	}
	return count
}

func copyGrid(grid [][]byte) [][]byte {
	g := make([][]byte, len(grid))
	for i, row := range grid {
		g[i] = make([]byte, len(row))
		copy(g[i], row)
	}
	return g
}

func part2(grid [][]byte) int {
	g := copyGrid(grid)
	total := 0

	for {
		// Find all accessible rolls in current state
		var toRemove [][2]int
		for r := range g {
			for c := range g[r] {
				if accessible(g, r, c) {
					toRemove = append(toRemove, [2]int{r, c})
				}
			}
		}
		if len(toRemove) == 0 {
			break
		}
		// Remove them all at once
		for _, pos := range toRemove {
			g[pos[0]][pos[1]] = '.'
		}
		total += len(toRemove)
	}
	return total
}

func main() {
	part := 0
	if len(os.Args) > 1 {
		fmt.Sscanf(os.Args[1], "%d", &part)
	}

	grid := readGrid("input.txt")

	if part == 1 || part == 0 {
		fmt.Printf("Part 1: %d\n", part1(grid))
	}
	if part == 2 || part == 0 {
		fmt.Printf("Part 2: %d\n", part2(grid))
	}
}
