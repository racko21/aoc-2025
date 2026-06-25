package main

import (
	"bufio"
	"fmt"
	"os"
	"sort"
	"strconv"
	"strings"
)

type point struct{ x, y, z int }

type edge struct {
	d    int64
	a, b int
}

var parent, rank_ []int

func find(x int) int {
	for parent[x] != x {
		parent[x] = parent[parent[x]]
		x = parent[x]
	}
	return x
}

func union(a, b int) bool {
	ra, rb := find(a), find(b)
	if ra == rb {
		return false
	}
	if rank_[ra] < rank_[rb] {
		ra, rb = rb, ra
	}
	parent[rb] = ra
	if rank_[ra] == rank_[rb] {
		rank_[ra]++
	}
	return true
}

func main() {
	part := 0
	if len(os.Args) > 1 {
		part, _ = strconv.Atoi(os.Args[1])
	}

	data, _ := os.ReadFile("input.txt")
	lines := strings.Split(strings.TrimSpace(string(data)), "\n")
	var pts []point
	for _, ln := range lines {
		ln = strings.TrimSpace(ln)
		if ln == "" {
			continue
		}
		f := strings.Split(ln, ",")
		x, _ := strconv.Atoi(strings.TrimSpace(f[0]))
		y, _ := strconv.Atoi(strings.TrimSpace(f[1]))
		z, _ := strconv.Atoi(strings.TrimSpace(f[2]))
		pts = append(pts, point{x, y, z})
	}

	n := len(pts)
	var edges []edge
	for i := 0; i < n; i++ {
		for j := i + 1; j < n; j++ {
			dx := int64(pts[i].x - pts[j].x)
			dy := int64(pts[i].y - pts[j].y)
			dz := int64(pts[i].z - pts[j].z)
			d := dx*dx + dy*dy + dz*dz
			edges = append(edges, edge{d, i, j})
		}
	}
	sort.Slice(edges, func(i, j int) bool {
		return edges[i].d < edges[j].d
	})

	w := bufio.NewWriter(os.Stdout)
	defer w.Flush()

	if part == 0 || part == 1 {
		parent = make([]int, n)
		rank_ = make([]int, n)
		for i := range parent {
			parent[i] = i
		}
		cnt := 0
		for _, e := range edges {
			if cnt >= 1000 {
				break
			}
			union(e.a, e.b)
			cnt++
		}
		sizes := map[int]int{}
		for i := 0; i < n; i++ {
			sizes[find(i)]++
		}
		var sz []int
		for _, v := range sizes {
			sz = append(sz, v)
		}
		sort.Sort(sort.Reverse(sort.IntSlice(sz)))
		res := 1
		for i := 0; i < 3 && i < len(sz); i++ {
			res *= sz[i]
		}
		fmt.Fprintf(w, "Part 1: %d\n", res)
	}

	if part == 0 || part == 2 {
		parent = make([]int, n)
		rank_ = make([]int, n)
		for i := range parent {
			parent[i] = i
		}
		comps := n
		var lastA, lastB int
		for _, e := range edges {
			if union(e.a, e.b) {
				comps--
				if comps == 1 {
					lastA, lastB = e.a, e.b
					break
				}
			}
		}
		res := pts[lastA].x * pts[lastB].x
		fmt.Fprintf(w, "Part 2: %d\n", res)
	}
}
