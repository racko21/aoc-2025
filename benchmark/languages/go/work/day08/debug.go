package main

import (
	"fmt"
	"math"
	"sort"
)

type Point struct {
	x, y, z int
}

type UF struct {
	parent []int
	size   []int
}

func newUF(n int) *UF {
	uf := &UF{parent: make([]int, n), size: make([]int, n)}
	for i := range uf.parent {
		uf.parent[i] = i
		uf.size[i] = 1
	}
	return uf
}

func (uf *UF) find(x int) int {
	for uf.parent[x] != x {
		uf.parent[x] = uf.parent[uf.parent[x]]
		x = uf.parent[x]
	}
	return x
}

func (uf *UF) union(a, b int) bool {
	ra, rb := uf.find(a), uf.find(b)
	if ra == rb {
		return false
	}
	if uf.size[ra] < uf.size[rb] {
		ra, rb = rb, ra
	}
	uf.parent[rb] = ra
	uf.size[ra] += uf.size[rb]
	return true
}

func dist(a, b Point) float64 {
	dx := float64(a.x - b.x)
	dy := float64(a.y - b.y)
	dz := float64(a.z - b.z)
	return math.Sqrt(dx*dx + dy*dy + dz*dz)
}

type Edge struct {
	i, j int
	d    float64
}

func main() {
	points := []Point{
		{162, 817, 812},
		{57, 618, 57},
		{906, 360, 560},
		{592, 479, 940},
		{352, 342, 300},
		{466, 668, 158},
		{542, 29, 236},
		{431, 825, 988},
		{739, 650, 466},
		{52, 470, 668},
		{216, 146, 977},
		{819, 987, 18},
		{117, 168, 530},
		{805, 96, 715},
		{346, 949, 466},
		{970, 615, 88},
		{941, 993, 340},
		{862, 61, 35},
		{984, 92, 344},
		{425, 690, 689},
	}

	n := len(points)
	var edges []Edge
	for i := 0; i < n; i++ {
		for j := i + 1; j < n; j++ {
			edges = append(edges, Edge{i, j, dist(points[i], points[j])})
		}
	}
	sort.Slice(edges, func(a, b int) bool {
		return edges[a].d < edges[b].d
	})

	fmt.Println("First 15 edges:")
	for k := 0; k < 15; k++ {
		e := edges[k]
		fmt.Printf("  %d: (%v) - (%v) = %.2f\n", k+1, points[e.i], points[e.j], e.d)
	}

	uf := newUF(n)
	fmt.Println("\nProcessing connections:")
	for k := 0; k < 10; k++ {
		e := edges[k]
		merged := uf.union(e.i, e.j)
		fmt.Printf("  Connection %d: %v - %v (merged=%v)\n", k+1, points[e.i], points[e.j], merged)
	}

	sizeMap := make(map[int]int)
	for i := 0; i < n; i++ {
		r := uf.find(i)
		sizeMap[r]++
	}
	sizes := []int{}
	for _, s := range sizeMap {
		sizes = append(sizes, s)
	}
	sort.Sort(sort.Reverse(sort.IntSlice(sizes)))
	fmt.Println("Circuit sizes:", sizes)
	result := 1
	for k := 0; k < 3 && k < len(sizes); k++ {
		result *= sizes[k]
	}
	fmt.Println("Product of top 3:", result)
}
