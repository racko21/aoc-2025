package main

import (
	"bufio"
	"fmt"
	"os"
	"sort"
	"strconv"
	"strings"
)

func main() {
	part := 0
	if len(os.Args) > 1 {
		part, _ = strconv.Atoi(os.Args[1])
	}

	f, err := os.Open("input.txt")
	if err != nil {
		panic(err)
	}
	defer f.Close()

	var reds [][2]int
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		parts := strings.Split(line, ",")
		x, _ := strconv.Atoi(parts[0])
		y, _ := strconv.Atoi(parts[1])
		reds = append(reds, [2]int{x, y})
	}

	if part == 0 || part == 1 {
		fmt.Println("Part 1:", part1(reds))
	}
	if part == 0 || part == 2 {
		fmt.Println("Part 2:", part2(reds))
	}
}

func part1(reds [][2]int) int {
	best := 0
	n := len(reds)
	for i := 0; i < n; i++ {
		for j := i + 1; j < n; j++ {
			dx := reds[i][0] - reds[j][0]
			if dx < 0 {
				dx = -dx
			}
			dy := reds[i][1] - reds[j][1]
			if dy < 0 {
				dy = -dy
			}
			area := (dx + 1) * (dy + 1)
			if area > best {
				best = area
			}
		}
	}
	return best
}

// For Part 2, we need to:
// 1. Compute all red+green tiles (polygon outline + interior)
// 2. Find the largest rectangle with red corners where all tiles are red/green
//
// Strategy:
// - Coordinate compress x and y values
// - In compressed space, mark cells that are entirely red/green
// - Use 2D prefix sums to check rectangles
// - For each pair of red tiles, check if the rectangle they define is fully covered

func part2(reds [][2]int) int {
	n := len(reds)

	// Collect all unique x and y coordinates from polygon edges
	// Each edge is horizontal or vertical between consecutive red tiles
	// We need to generate all relevant x/y coordinates for compression

	// Gather all x and y coordinates
	xSet := map[int]bool{}
	ySet := map[int]bool{}
	for _, r := range reds {
		xSet[r[0]] = true
		ySet[r[1]] = true
	}

	// For edges between consecutive red tiles:
	// The edges are axis-aligned, so we just need the endpoint coordinates
	// (already included above)

	// Build sorted unique coordinate arrays
	xs := sortedKeys(xSet)
	ys := sortedKeys(ySet)

	// Coordinate compression maps
	xIdx := map[int]int{}
	yIdx := map[int]int{}
	for i, v := range xs {
		xIdx[v] = i
	}
	for i, v := range ys {
		yIdx[v] = i
	}

	nx := len(xs)
	ny := len(ys)

	// Build a grid of "cells" between compressed coordinates.
	// We use a cell-based approach: cell (i,j) represents the region
	// [xs[i], xs[i+1]-1] x [ys[j], ys[j+1]-1] in real coordinates.
	// But for simplicity, let's think of the actual tile grid.
	//
	// Actually, let's think differently. Since all edges are axis-aligned and
	// endpoints are on our compressed coordinates, we can use a compressed grid
	// where each "cell" represents the block of tiles between consecutive
	// compressed coordinates.
	//
	// A compressed cell (i,j) for 0 <= i < nx-1, 0 <= j < ny-1 represents
	// real tiles x in [xs[i], xs[i+1]-1], y in [ys[j], ys[j+1]-1].
	// The "point" cell (i,j) where both are actual coordinates represents
	// exactly tile (xs[i], ys[j]).
	//
	// This gets complex. Let me use a simpler approach:
	// Since the polygon edges are axis-aligned, I'll use a different compressed
	// grid that captures both the edge lines and the gaps between them.
	//
	// Alternative: expand coordinates by 2 (2*x, 2*y) so we can represent
	// both tiles and half-gaps. But let's try a cleaner approach.

	// Simpler approach: use the "odd-even" rule.
	// For each compressed cell, determine if it's inside/on the polygon.
	// A compressed cell (ci, cj) corresponds to real coordinate (xs[ci], ys[cj]).
	// Wait - each compressed index is just a specific coordinate.
	// The "area" between consecutive coordinates we handle separately.

	// Let me think again. The compressed grid has nx * ny "points" where
	// each point is a real tile coordinate. Between consecutive compressed
	// x values, there may be many real tiles. Let's call those "gap cells".

	// The key insight: since all polygon edges are axis-aligned and their
	// endpoints are in our coordinate set, a horizontal strip between y[j] and y[j+1]
	// (for consecutive compressed y values) is entirely inside or outside the
	// polygon (except for the edge lines themselves).

	// I'll use a compressed grid with "segments" between coordinates:
	// - Pixel at compressed coords (ci, cj) = real tile (xs[ci], ys[cj])
	// - Horizontal segment between (ci, cj) and (ci+1, cj) = real tiles from xs[ci]+1 to xs[ci+1]-1 at y=ys[cj]
	// - Vertical segment between (ci, cj) and (ci, cj+1) = real tiles from ys[cj]+1 to ys[cj+1]-1 at x=xs[ci]
	// - Block between (ci,cj),(ci+1,cj),(ci,cj+1),(ci+1,cj+1) = interior block

	// This gives a 2*(nx) - 1 by 2*(ny) - 1 expanded compressed grid.

	// Let me use this expanded grid approach.
	// Expanded grid size: (2*nx-1) x (2*ny-1)
	// Mapping: real coordinate index (ci, cj) -> expanded index (2*ci, 2*cj)
	// Segment (ci, ci+1) at row cj -> expanded (2*ci+1, 2*cj)
	// Block -> expanded (2*ci+1, 2*cj+1)

	egx := 2*nx - 1
	egy := 2*ny - 1

	// Build edge set: mark which expanded cells are ON the polygon boundary
	edge := make([][]bool, egx)
	for i := range edge {
		edge[i] = make([]bool, egy)
	}

	// Mark red tiles
	for _, r := range reds {
		ci := xIdx[r[0]]
		cj := yIdx[r[1]]
		edge[2*ci][2*cj] = true
	}

	// Mark green tiles on edges (between consecutive red tiles)
	for i := 0; i < n; i++ {
		a := reds[i]
		b := reds[(i+1)%n]
		if a[0] == b[0] {
			// vertical edge
			ci := xIdx[a[0]]
			j1 := yIdx[a[1]]
			j2 := yIdx[b[1]]
			if j1 > j2 {
				j1, j2 = j2, j1
			}
			// Mark all tiles from ys[j1] to ys[j2] at x=xs[ci]
			// In expanded grid: x=2*ci, y from 2*j1 to 2*j2
			for ej := 2*j1 + 1; ej < 2*j2; ej++ {
				edge[2*ci][ej] = true
			}
		} else if a[1] == b[1] {
			// horizontal edge
			cj := yIdx[a[1]]
			i1 := xIdx[a[0]]
			i2 := xIdx[b[0]]
			if i1 > i2 {
				i1, i2 = i2, i1
			}
			// Mark all tiles from xs[i1] to xs[i2] at y=ys[cj]
			// In expanded grid: y=2*cj, x from 2*i1 to 2*i2
			for ei := 2*i1 + 1; ei < 2*i2; ei++ {
				edge[ei][2*cj] = true
			}
		}
	}

	// Now flood fill from outside to find all cells that are NOT inside the polygon
	// Use BFS from corners of the expanded grid
	inside := make([][]bool, egx)
	for i := range inside {
		inside[i] = make([]bool, egy)
	}

	// outside[i][j] = true means this expanded cell is outside (or on boundary but reachable from outside)
	outside := make([][]bool, egx)
	for i := range outside {
		outside[i] = make([]bool, egy)
	}

	type point struct{ x, y int }
	queue := []point{}

	enqueue := func(x, y int) {
		if x < 0 || x >= egx || y < 0 || y >= egy {
			return
		}
		if outside[x][y] || edge[x][y] {
			return
		}
		outside[x][y] = true
		queue = append(queue, point{x, y})
	}

	// Start flood fill from all border cells
	for i := 0; i < egx; i++ {
		enqueue(i, 0)
		enqueue(i, egy-1)
	}
	for j := 0; j < egy; j++ {
		enqueue(0, j)
		enqueue(egx-1, j)
	}

	dx := []int{0, 0, 1, -1}
	dy := []int{1, -1, 0, 0}

	for len(queue) > 0 {
		p := queue[0]
		queue = queue[1:]
		for d := 0; d < 4; d++ {
			nx2 := p.x + dx[d]
			ny2 := p.y + dy[d]
			enqueue(nx2, ny2)
		}
	}

	// A cell in expanded grid is "red or green" if it's an edge cell OR it's inside (not outside, not unreachable non-edge)
	// inside = !outside && !edge -> interior (green)
	// edge -> red or green (on boundary)
	// So red-or-green = edge OR (!outside && !edge) = !outside
	// Wait: edge cells are NOT marked as outside (we skip them in enqueue).
	// So a cell is red-or-green if: edge[x][y] OR (!outside[x][y] && !edge[x][y])
	// Which simplifies to: !outside[x][y] (since edge cells are not outside)
	// But wait: edge cells might not be outside, and non-edge non-outside cells are inside.
	// So: red-or-green = !outside[x][y]

	for i := 0; i < egx; i++ {
		for j := 0; j < egy; j++ {
			inside[i][j] = !outside[i][j]
		}
	}

	// Now I need to check: for a rectangle from (x1,y1) to (x2,y2) in real coordinates,
	// where (x1,y1) and (x2,y2) are both red tiles, is every tile in the rectangle inside?
	//
	// A rectangle in real space spans compressed indices:
	// x: from xIdx[x1] to xIdx[x2] (or vice versa)
	// y: from yIdx[y1] to yIdx[y2] (or vice versa)
	// In expanded space: x from 2*ci1 to 2*ci2, y from 2*cj1 to 2*cj2
	// (inclusive)
	//
	// For the rectangle to be all red/green, all expanded cells in that range must be inside.
	// We can use a 2D prefix sum on the "bad" cells (outside cells).

	// Build prefix sum of "bad" cells (cells that are outside)
	bad := make([][]int, egx+1)
	for i := range bad {
		bad[i] = make([]int, egy+1)
	}
	for i := 0; i < egx; i++ {
		for j := 0; j < egy; j++ {
			v := 0
			if outside[i][j] {
				v = 1
			}
			bad[i+1][j+1] = v + bad[i][j+1] + bad[i+1][j] - bad[i][j]
		}
	}

	queryBad := func(x1, y1, x2, y2 int) int {
		// Query sum of bad cells in [x1..x2] x [y1..y2] (0-indexed, inclusive)
		if x1 > x2 || y1 > y2 {
			return 0
		}
		return bad[x2+1][y2+1] - bad[x1][y2+1] - bad[x2+1][y1] + bad[x1][y1]
	}

	// For each pair of red tiles, compute the rectangle and check if valid
	best := 0
	for i := 0; i < n; i++ {
		for j := i + 1; j < n; j++ {
			r1 := reds[i]
			r2 := reds[j]

			// Real coordinates of rectangle corners
			minX, maxX := r1[0], r2[0]
			if minX > maxX {
				minX, maxX = maxX, minX
			}
			minY, maxY := r1[1], r2[1]
			if minY > maxY {
				minY, maxY = maxY, minY
			}

			if minX == maxX || minY == maxY {
				// Degenerate rectangle (line) - area is (maxX-minX+1)*(maxY-minY+1)
				// Still valid as long as all tiles are red/green
				// Check using expanded grid
			}

			// Expanded grid indices
			ci1 := xIdx[minX]
			ci2 := xIdx[maxX]
			cj1 := yIdx[minY]
			cj2 := yIdx[maxY]

			ex1 := 2 * ci1
			ex2 := 2 * ci2
			ey1 := 2 * cj1
			ey2 := 2 * cj2

			// Check if all cells in expanded rectangle are "inside" (not outside)
			if queryBad(ex1, ey1, ex2, ey2) == 0 {
				// Valid rectangle
				area := (maxX - minX + 1) * (maxY - minY + 1)
				if area > best {
					best = area
				}
			}
		}
	}

	return best
}

func sortedKeys(m map[int]bool) []int {
	keys := make([]int, 0, len(m))
	for k := range m {
		keys = append(keys, k)
	}
	sort.Ints(keys)
	return keys
}

func abs(x int) int {
	if x < 0 {
		return -x
	}
	return x
}

var _ = abs // suppress unused warning
