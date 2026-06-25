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

	uf := newUF(n)
	fmt.Println("All MST edges (Kruskal's):")
	for _, e := range edges {
		merged := uf.union(e.i, e.j)
		if merged {
			fmt.Printf("  %v - %v (dist=%.2f)\n", points[e.i], points[e.j], e.d)
		}
	}
	
	// Prim's approach to find max MST edge
	const INF = math.MaxFloat64
	inMST := make([]bool, n)
	minDist := make([]float64, n)
	closestMST := make([]int, n)
	for i := range minDist {
		minDist[i] = INF
		closestMST[i] = -1
	}
	inMST[0] = true
	for j := 1; j < n; j++ {
		minDist[j] = dist(points[0], points[j])
		closestMST[j] = 0
	}

	var lastA, lastB int
	var lastD float64 = -1

	fmt.Println("\nPrim's MST edges:")
	for step := 0; step < n-1; step++ {
		minVal := INF
		u := -1
		for i := 0; i < n; i++ {
			if !inMST[i] && minDist[i] < minVal {
				minVal = minDist[i]
				u = i
			}
		}
		if u == -1 {
			break
		}
		fmt.Printf("  Step %d: %v - %v (dist=%.2f)\n", step+1, points[closestMST[u]], points[u], minDist[u])
		if minDist[u] > lastD {
			lastD = minDist[u]
			lastA = closestMST[u]
			lastB = u
		}
		inMST[u] = true
		for v := 0; v < n; v++ {
			if !inMST[v] {
				d := dist(points[u], points[v])
				if d < minDist[v] {
					minDist[v] = d
					closestMST[v] = u
				}
			}
		}
	}
	
	fmt.Printf("\nMax MST edge (Prim's): %v - %v (dist=%.2f)\n", points[lastA], points[lastB], lastD)
	fmt.Printf("Product: %d\n", points[lastA].x * points[lastB].x)
}
