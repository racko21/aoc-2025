// Day 8: Kruskal-style union-find on 3D junction boxes.
// Part 1: multiply top-3 component sizes after 1000 connections.
// Part 2: multiply X coords of the two nodes in the final merging edge.
package main

import (
	"fmt"
	"sort"
	"strings"

	"github.com/racko21/aoc-2025/utils"
)

type point struct{ x, y, z int }

func parse(path string) []point {
	var pts []point
	for _, line := range utils.ReadLines(path) {
		if line == "" {
			continue
		}
		p := strings.SplitN(line, ",", 3)
		pts = append(pts, point{utils.Atoi(p[0]), utils.Atoi(p[1]), utils.Atoi(p[2])})
	}
	return pts
}

func dist2(a, b point) int64 {
	dx, dy, dz := int64(a.x-b.x), int64(a.y-b.y), int64(a.z-b.z)
	return dx*dx + dy*dy + dz*dz
}

type edge struct {
	i, j int
	d2   int64
}

func buildEdges(pts []point) []edge {
	n := len(pts)
	edges := make([]edge, 0, n*(n-1)/2)
	for i := 0; i < n; i++ {
		for j := i + 1; j < n; j++ {
			edges = append(edges, edge{i, j, dist2(pts[i], pts[j])})
		}
	}
	sort.Slice(edges, func(a, b int) bool { return edges[a].d2 < edges[b].d2 })
	return edges
}

type uf struct{ parent, size []int }

func newUF(n int) *uf {
	u := &uf{make([]int, n), make([]int, n)}
	for i := range u.parent {
		u.parent[i] = i
		u.size[i] = 1
	}
	return u
}

func (u *uf) find(x int) int {
	for u.parent[x] != x {
		u.parent[x] = u.parent[u.parent[x]]
		x = u.parent[x]
	}
	return x
}

func (u *uf) union(x, y int) bool {
	rx, ry := u.find(x), u.find(y)
	if rx == ry {
		return false
	}
	if u.size[rx] < u.size[ry] {
		rx, ry = ry, rx
	}
	u.parent[ry] = rx
	u.size[rx] += u.size[ry]
	return true
}

// solve connects the k closest pairs and returns product of 3 largest component sizes.
func solve(path string, k int) int {
	pts := parse(path)
	edges := buildEdges(pts)
	u := newUF(len(pts))
	limit := k
	if limit > len(edges) {
		limit = len(edges)
	}
	for i := 0; i < limit; i++ {
		u.union(edges[i].i, edges[i].j)
	}
	sizes := make(map[int]int)
	for i := range pts {
		sizes[u.find(i)]++
	}
	sl := make([]int, 0, len(sizes))
	for _, s := range sizes {
		sl = append(sl, s)
	}
	sort.Sort(sort.Reverse(sort.IntSlice(sl)))
	return sl[0] * sl[1] * sl[2]
}

// lastMerge returns the product of X coords of the two nodes whose connection
// finally unifies all junction boxes into one circuit.
func lastMerge(path string) int {
	pts := parse(path)
	edges := buildEdges(pts)
	u := newUF(len(pts))
	components := len(pts)
	for _, e := range edges {
		if u.union(e.i, e.j) {
			components--
			if components == 1 {
				return pts[e.i].x * pts[e.j].x
			}
		}
	}
	return -1
}

func part1(path string) int { return solve(path, 1000) }
func part2(path string) int { return lastMerge(path) }

func main() {
	fmt.Println("Part 1:", part1("input.txt"))
	fmt.Println("Part 2:", part2("input.txt"))
}
