// Day 9: largest rectangle with red tile opposite corners.
// Part 1: maximize area over all pairs. Part 2: rectangle must lie entirely within the polygon.
package main

import (
	"fmt"
	"sort"
	"strings"

	"github.com/racko21/aoc-2025/utils"
)

type point struct{ x, y int }

func parse(path string) []point {
	var pts []point
	for _, line := range utils.ReadLines(path) {
		if line == "" {
			continue
		}
		p := strings.SplitN(line, ",", 2)
		pts = append(pts, point{utils.Atoi(p[0]), utils.Atoi(p[1])})
	}
	return pts
}

func abs(x int) int {
	if x < 0 {
		return -x
	}
	return x
}

func part1(path string) int {
	pts := parse(path)
	best := 0
	for i := 0; i < len(pts); i++ {
		for j := i + 1; j < len(pts); j++ {
			area := (abs(pts[j].x-pts[i].x) + 1) * (abs(pts[j].y-pts[i].y) + 1)
			if area > best {
				best = area
			}
		}
	}
	return best
}

func part2(path string) int {
	pts := parse(path)
	n := len(pts)

	// Coordinate compression: collect unique x/y, map to odd indices (gap cells = even).
	xm, ym := map[int]struct{}{}, map[int]struct{}{}
	for _, p := range pts {
		xm[p.x] = struct{}{}
		ym[p.y] = struct{}{}
	}
	toSlice := func(m map[int]struct{}) []int {
		s := make([]int, 0, len(m))
		for k := range m {
			s = append(s, k)
		}
		sort.Ints(s)
		return s
	}
	xs, ys := toSlice(xm), toSlice(ym)
	W, H := 2*len(xs)+1, 2*len(ys)+1

	xIdx, yIdx := map[int]int{}, map[int]int{}
	for i, x := range xs {
		xIdx[x] = 2*i + 1
	}
	for i, y := range ys {
		yIdx[y] = 2*i + 1
	}

	// Draw polygon boundary on compressed grid.
	bnd := make([][]bool, H)
	for i := range bnd {
		bnd[i] = make([]bool, W)
	}
	for i := 0; i < n; i++ {
		a, b := pts[i], pts[(i+1)%n]
		ax, ay := xIdx[a.x], yIdx[a.y]
		bx, by := xIdx[b.x], yIdx[b.y]
		if ax == bx { // vertical
			if ay > by {
				ay, by = by, ay
			}
			for cy := ay; cy <= by; cy++ {
				bnd[cy][ax] = true
			}
		} else { // horizontal
			if ax > bx {
				ax, bx = bx, ax
			}
			for cx := ax; cx <= bx; cx++ {
				bnd[ay][cx] = true
			}
		}
	}

	// BFS flood-fill from (0,0) to mark all "outside" cells.
	out := make([][]bool, H)
	for i := range out {
		out[i] = make([]bool, W)
	}
	type pos struct{ x, y int }
	q := []pos{{0, 0}}
	out[0][0] = true
	dirs := [4][2]int{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}
	for len(q) > 0 {
		p := q[0]
		q = q[1:]
		for _, d := range dirs {
			nx, ny := p.x+d[0], p.y+d[1]
			if nx < 0 || nx >= W || ny < 0 || ny >= H || out[ny][nx] || bnd[ny][nx] {
				continue
			}
			out[ny][nx] = true
			q = append(q, pos{nx, ny})
		}
	}

	// 2D prefix sum of outside cells for O(1) rectangle queries.
	ps := make([][]int, H+1)
	for i := range ps {
		ps[i] = make([]int, W+1)
	}
	for cy := 0; cy < H; cy++ {
		for cx := 0; cx < W; cx++ {
			v := 0
			if out[cy][cx] {
				v = 1
			}
			ps[cy+1][cx+1] = v + ps[cy][cx+1] + ps[cy+1][cx] - ps[cy][cx]
		}
	}
	query := func(cx1, cy1, cx2, cy2 int) int {
		return ps[cy2+1][cx2+1] - ps[cy1][cx2+1] - ps[cy2+1][cx1] + ps[cy1][cx1]
	}

	// Check all pairs of red tiles as rectangle corners.
	best := 0
	for i := 0; i < n; i++ {
		for j := i + 1; j < n; j++ {
			x1, y1 := pts[i].x, pts[i].y
			x2, y2 := pts[j].x, pts[j].y
			if x1 > x2 {
				x1, x2 = x2, x1
			}
			if y1 > y2 {
				y1, y2 = y2, y1
			}
			if query(xIdx[x1], yIdx[y1], xIdx[x2], yIdx[y2]) == 0 {
				if area := (x2 - x1 + 1) * (y2 - y1 + 1); area > best {
					best = area
				}
			}
		}
	}
	return best
}

func main() {
	fmt.Println("Part 1:", part1("input.txt"))
	fmt.Println("Part 2:", part2("input.txt"))
}
