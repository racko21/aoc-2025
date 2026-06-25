package main

import (
	"bufio"
	"container/heap"
	"fmt"
	"math"
	"os"
	"sort"
	"strconv"
	"strings"
)

type Point struct {
	x, y, z int
}

// Union-Find
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

func (uf *UF) numComponents() int {
	count := 0
	for i := range uf.parent {
		if uf.find(i) == i {
			count++
		}
	}
	return count
}

func distSq(a, b Point) int64 {
	dx := int64(a.x - b.x)
	dy := int64(a.y - b.y)
	dz := int64(a.z - b.z)
	return dx*dx + dy*dy + dz*dz
}

type Edge struct {
	i, j int
	dsq  int64
}

// MaxHeap of edges (by distance squared) - for keeping k smallest
type EdgeMaxHeap []Edge

func (h EdgeMaxHeap) Len() int           { return len(h) }
func (h EdgeMaxHeap) Less(i, j int) bool { return h[i].dsq > h[j].dsq } // max at top
func (h EdgeMaxHeap) Swap(i, j int)      { h[i], h[j] = h[j], h[i] }
func (h *EdgeMaxHeap) Push(x interface{}) {
	*h = append(*h, x.(Edge))
}
func (h *EdgeMaxHeap) Pop() interface{} {
	old := *h
	n := len(old)
	x := old[n-1]
	*h = old[:n-1]
	return x
}

func readPoints(filename string) []Point {
	f, err := os.Open(filename)
	if err != nil {
		panic(err)
	}
	defer f.Close()

	var points []Point
	scanner := bufio.NewScanner(f)
	scanner.Buffer(make([]byte, 1024*1024), 1024*1024)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		parts := strings.Split(line, ",")
		x, _ := strconv.Atoi(parts[0])
		y, _ := strconv.Atoi(parts[1])
		z, _ := strconv.Atoi(parts[2])
		points = append(points, Point{x, y, z})
	}
	return points
}

// getAllEdgesSorted returns all edges sorted by distance squared, then by (i, j)
func getAllEdgesSorted(points []Point) []Edge {
	n := len(points)
	total := n * (n - 1) / 2
	edges := make([]Edge, 0, total)
	for i := 0; i < n; i++ {
		for j := i + 1; j < n; j++ {
			edges = append(edges, Edge{i, j, distSq(points[i], points[j])})
		}
	}
	sort.Slice(edges, func(a, b int) bool {
		if edges[a].dsq != edges[b].dsq {
			return edges[a].dsq < edges[b].dsq
		}
		if edges[a].i != edges[b].i {
			return edges[a].i < edges[b].i
		}
		return edges[a].j < edges[b].j
	})
	return edges
}

func part1(points []Point) int {
	n := len(points)
	const K = 1000

	uf := newUF(n)
	totalPairs := n * (n - 1) / 2

	if totalPairs <= K {
		// Process all pairs sorted by distance
		edges := getAllEdgesSorted(points)
		for _, e := range edges {
			uf.union(e.i, e.j)
		}
	} else if n <= 6000 {
		// O(n^2) sorting feasible
		edges := getAllEdgesSorted(points)
		for k := 0; k < K; k++ {
			e := edges[k]
			uf.union(e.i, e.j)
		}
	} else {
		// Keep K smallest edges using a max-heap of size K
		h := &EdgeMaxHeap{}
		heap.Init(h)
		for i := 0; i < n; i++ {
			for j := i + 1; j < n; j++ {
				e := Edge{i, j, distSq(points[i], points[j])}
				if h.Len() < K {
					heap.Push(h, e)
				} else if e.dsq < (*h)[0].dsq {
					heap.Pop(h)
					heap.Push(h, e)
				}
			}
		}
		// Sort the K edges by distance and process
		edges := []Edge(*h)
		sort.Slice(edges, func(a, b int) bool {
			if edges[a].dsq != edges[b].dsq {
				return edges[a].dsq < edges[b].dsq
			}
			if edges[a].i != edges[b].i {
				return edges[a].i < edges[b].i
			}
			return edges[a].j < edges[b].j
		})
		for _, e := range edges {
			uf.union(e.i, e.j)
		}
	}

	// Find component sizes
	sizeMap := make(map[int]int)
	for i := 0; i < n; i++ {
		r := uf.find(i)
		sizeMap[r]++
	}
	sizes := make([]int, 0, len(sizeMap))
	for _, s := range sizeMap {
		sizes = append(sizes, s)
	}
	sort.Sort(sort.Reverse(sort.IntSlice(sizes)))
	result := 1
	for k := 0; k < 3 && k < len(sizes); k++ {
		result *= sizes[k]
	}
	return result
}

func part2(points []Point) int {
	n := len(points)

	if n == 1 {
		return points[0].x * points[0].x
	}

	// Kruskal's MST: process edges in order, track last merge
	// For large n, use Prim's O(n^2) to avoid O(n^2 log n) memory
	
	if n <= 6000 {
		// Can afford O(n^2) edges in memory
		edges := getAllEdgesSorted(points)
		uf := newUF(n)
		var lastA, lastB int
		for _, e := range edges {
			if uf.union(e.i, e.j) {
				lastA, lastB = e.i, e.j
				if uf.numComponents() == 1 {
					break
				}
			}
		}
		return points[lastA].x * points[lastB].x
	}

	// Use Prim's algorithm to find MST max-weight edge
	// This finds the same answer as Kruskal's (max weight MST edge = last Kruskal merge)
	const INF = int64(math.MaxInt64)
	inMST := make([]bool, n)
	minDist := make([]int64, n)
	closestMSTNode := make([]int, n)

	for i := range minDist {
		minDist[i] = INF
		closestMSTNode[i] = -1
	}

	inMST[0] = true
	for j := 1; j < n; j++ {
		minDist[j] = distSq(points[0], points[j])
		closestMSTNode[j] = 0
	}

	var lastEdgeA, lastEdgeB int
	var lastEdgeDist int64 = -1

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

		if minDist[u] > lastEdgeDist {
			lastEdgeDist = minDist[u]
			lastEdgeA = closestMSTNode[u]
			lastEdgeB = u
		}

		inMST[u] = true

		for v := 0; v < n; v++ {
			if !inMST[v] {
				d := distSq(points[u], points[v])
				if d < minDist[v] {
					minDist[v] = d
					closestMSTNode[v] = u
				}
			}
		}
	}

	return points[lastEdgeA].x * points[lastEdgeB].x
}

func main() {
	part := 0
	if len(os.Args) > 1 {
		switch os.Args[1] {
		case "1":
			part = 1
		case "2":
			part = 2
		}
	}

	points := readPoints("input.txt")

	if part == 1 || part == 0 {
		fmt.Printf("Part 1: %d\n", part1(points))
	}
	if part == 2 || part == 0 {
		fmt.Printf("Part 2: %d\n", part2(points))
	}
}
