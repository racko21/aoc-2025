// Polyomino packing: count how many regions can fit all required present shapes
// without '#' cells overlapping. Shapes can be rotated/flipped; '.' is transparent.
package main

import (
	"fmt"
	"sort"
	"strings"

	utils "github.com/racko21/aoc-2025/utils"
)

type Pt struct{ r, c int }

// normalizeShape translates shape so min row=0, min col=0, then sorts.
func normalizeShape(pts []Pt) []Pt {
	if len(pts) == 0 {
		return pts
	}
	minR, minC := pts[0].r, pts[0].c
	for _, p := range pts {
		if p.r < minR {
			minR = p.r
		}
		if p.c < minC {
			minC = p.c
		}
	}
	out := make([]Pt, len(pts))
	for i, p := range pts {
		out[i] = Pt{p.r - minR, p.c - minC}
	}
	sort.Slice(out, func(i, j int) bool {
		if out[i].r != out[j].r {
			return out[i].r < out[j].r
		}
		return out[i].c < out[j].c
	})
	return out
}

func rotCW(pts []Pt) []Pt {
	out := make([]Pt, len(pts))
	for i, p := range pts {
		out[i] = Pt{p.c, -p.r}
	}
	return normalizeShape(out)
}

func flipH(pts []Pt) []Pt {
	out := make([]Pt, len(pts))
	for i, p := range pts {
		out[i] = Pt{p.r, -p.c}
	}
	return normalizeShape(out)
}

func shapeKey(pts []Pt) string {
	var sb strings.Builder
	for _, p := range pts {
		fmt.Fprintf(&sb, "%d,%d;", p.r, p.c)
	}
	return sb.String()
}

// allOrientations returns up to 8 unique orientations.
func allOrientations(pts []Pt) [][]Pt {
	seen := map[string]bool{}
	var result [][]Pt
	cur := normalizeShape(pts)
	for f := 0; f < 2; f++ {
		for r := 0; r < 4; r++ {
			k := shapeKey(cur)
			if !seen[k] {
				seen[k] = true
				result = append(result, cur)
			}
			cur = rotCW(cur)
		}
		cur = flipH(cur)
	}
	return result
}

// canPlace checks if shape oriented as `pts` fits at top-left (dr, dc) in grid.
func canPlace(grid [][]bool, H, W int, pts []Pt, dr, dc int) bool {
	for _, p := range pts {
		r, c := p.r+dr, p.c+dc
		if r < 0 || r >= H || c < 0 || c >= W || grid[r][c] {
			return false
		}
	}
	return true
}

func place(grid [][]bool, pts []Pt, dr, dc int, val bool) {
	for _, p := range pts {
		grid[p.r+dr][p.c+dc] = val
	}
}

// solve tries to place all presents. presents is a list of (orientations, count).
// placedPos[i] tracks the minimum starting position for the i-th present type to avoid duplicates.
func solve(grid [][]bool, H, W int, presents [][][]Pt, counts []int, idx int, minPos []int) bool {
	// advance idx to next type with remaining count
	for idx < len(presents) && counts[idx] == 0 {
		idx++
	}
	if idx == len(presents) {
		return true
	}

	orientations := presents[idx]
	start := minPos[idx]

	for pos := start; pos < H*W; pos++ {
		dr, dc := pos/W, pos%W
		for _, orient := range orientations {
			if !canPlace(grid, H, W, orient, dr, dc) {
				continue
			}
			place(grid, orient, dr, dc, true)
			counts[idx]--
			saved := minPos[idx]
			minPos[idx] = pos // next copy must go at >= pos
			if solve(grid, H, W, presents, counts, idx, minPos) {
				return true
			}
			minPos[idx] = saved
			counts[idx]++
			place(grid, orient, dr, dc, false)
		}
	}
	return false
}

func canFit(W, H int, shapeOrientations [][][]Pt, shapeCells []int, counts []int) bool {
	totalPresents := 0
	totalCells := 0
	for i, c := range counts {
		totalPresents += c
		totalCells += c * shapeCells[i]
	}
	// Necessary condition: enough cells in the region
	if totalCells > W*H {
		return false
	}
	// All shapes have 3×3 bounding boxes; if there are enough non-overlapping
	// aligned 3×3 slots, we can trivially assign one present per slot.
	alignedSlots := (W / 3) * (H / 3)
	if totalPresents <= alignedSlots {
		return true
	}
	// Fallback to backtracking for tight cases
	grid := make([][]bool, H)
	for i := range grid {
		grid[i] = make([]bool, W)
	}
	minPos := make([]int, len(counts))
	countsCopy := make([]int, len(counts))
	copy(countsCopy, counts)
	return solve(grid, H, W, shapeOrientations, countsCopy, 0, minPos)
}

func parseShapes(lines []string) ([][][]Pt, []int) {
	var shapeBlocks [][]string
	var cur []string
	inShapes := true
	for _, line := range lines {
		if !inShapes {
			break
		}
		if len(line) > 0 && strings.Contains(line, "x") && strings.Contains(line, ":") {
			inShapes = false
			break
		}
		if line == "" {
			if len(cur) > 0 {
				shapeBlocks = append(shapeBlocks, cur)
				cur = nil
			}
		} else {
			cur = append(cur, line)
		}
	}
	if len(cur) > 0 {
		shapeBlocks = append(shapeBlocks, cur)
	}

	var orientations [][][]Pt
	var cellCounts []int
	for _, block := range shapeBlocks {
		var pts []Pt
		for r, row := range block[1:] {
			for c, ch := range row {
				if ch == '#' {
					pts = append(pts, Pt{r, c})
				}
			}
		}
		orientations = append(orientations, allOrientations(pts))
		cellCounts = append(cellCounts, len(pts))
	}
	return orientations, cellCounts
}

type Region struct {
	W, H   int
	counts []int
}

func parseRegions(lines []string, numShapes int) []Region {
	var regions []Region
	for _, line := range lines {
		if !strings.Contains(line, "x") || !strings.Contains(line, ":") {
			continue
		}
		// "WxH: c0 c1 c2 ..."
		colon := strings.Index(line, ":")
		dim := line[:colon]
		rest := strings.TrimSpace(line[colon+1:])
		parts := strings.SplitN(dim, "x", 2)
		W := utils.Atoi(parts[0])
		H := utils.Atoi(parts[1])
		counts := utils.SplitInts(rest, " ")
		regions = append(regions, Region{W, H, counts})
	}
	return regions
}

func main() {
	lines := utils.ReadLines("input.txt")
	shapeOrientations, shapeCells := parseShapes(lines)
	regions := parseRegions(lines, len(shapeOrientations))

	count := 0
	for _, reg := range regions {
		if canFit(reg.W, reg.H, shapeOrientations, shapeCells, reg.counts) {
			count++
		}
	}
	fmt.Println("Part 1:", count)
}
