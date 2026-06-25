package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// A shape is represented as a list of (row, col) offsets relative to top-left of bounding box
type Shape struct {
	cells [][]int // each element is [row, col]
	rows  int
	cols  int
}

func parseShapes(lines []string) []Shape {
	var shapes []Shape
	i := 0
	for i < len(lines) {
		line := strings.TrimSpace(lines[i])
		// check for shape header like "0:", "5:", etc - number followed by colon, no 'x'
		if len(line) >= 2 && line[len(line)-1] == ':' && !strings.Contains(line, "x") {
			numStr := line[:len(line)-1]
			_, err := strconv.Atoi(numStr)
			if err == nil {
				i++
				var grid []string
				for i < len(lines) {
					l := lines[i]
					ls := strings.TrimSpace(l)
					if ls == "" {
						i++
						break
					}
					grid = append(grid, l)
					i++
				}
				sh := parseGrid(grid)
				shapes = append(shapes, sh)
				continue
			}
		}
		i++
	}
	return shapes
}

func parseGrid(grid []string) Shape {
	var cells [][]int
	rows := len(grid)
	cols := 0
	for r, row := range grid {
		if len(row) > cols {
			cols = len(row)
		}
		for c, ch := range row {
			if ch == '#' {
				cells = append(cells, []int{r, c})
			}
		}
	}
	return Shape{cells: cells, rows: rows, cols: cols}
}

func normalize(cells [][]int) [][]int {
	if len(cells) == 0 {
		return cells
	}
	minR, minC := cells[0][0], cells[0][1]
	for _, c := range cells {
		if c[0] < minR {
			minR = c[0]
		}
		if c[1] < minC {
			minC = c[1]
		}
	}
	out := make([][]int, len(cells))
	for i, c := range cells {
		out[i] = []int{c[0] - minR, c[1] - minC}
	}
	sortCells(out)
	return out
}

func sortCells(cells [][]int) {
	n := len(cells)
	for i := 0; i < n; i++ {
		for j := i + 1; j < n; j++ {
			if cells[j][0] < cells[i][0] || (cells[j][0] == cells[i][0] && cells[j][1] < cells[i][1]) {
				cells[i], cells[j] = cells[j], cells[i]
			}
		}
	}
}

func rotate90(cells [][]int) [][]int {
	// (r,c) -> (c, -r) then normalize
	out := make([][]int, len(cells))
	for i, c := range cells {
		out[i] = []int{c[1], -c[0]}
	}
	return normalize(out)
}

func flipH(cells [][]int) [][]int {
	// (r,c) -> (r,-c) then normalize
	out := make([][]int, len(cells))
	for i, c := range cells {
		out[i] = []int{c[0], -c[1]}
	}
	return normalize(out)
}

func cellsKey(cells [][]int) string {
	var sb strings.Builder
	for i, c := range cells {
		if i > 0 {
			sb.WriteByte(';')
		}
		sb.WriteString(strconv.Itoa(c[0]))
		sb.WriteByte(',')
		sb.WriteString(strconv.Itoa(c[1]))
	}
	return sb.String()
}

// Generate all unique orientations of a shape
func allOrientations(s Shape) []Shape {
	cells := normalize(s.cells)
	seen := map[string]bool{}
	var result []Shape

	cur := cells
	for f := 0; f < 2; f++ {
		for rot := 0; rot < 4; rot++ {
			key := cellsKey(cur)
			if !seen[key] {
				seen[key] = true
				maxR, maxC := 0, 0
				for _, c := range cur {
					if c[0] > maxR {
						maxR = c[0]
					}
					if c[1] > maxC {
						maxC = c[1]
					}
				}
				result = append(result, Shape{cells: cur, rows: maxR + 1, cols: maxC + 1})
			}
			cur = rotate90(cur)
		}
		if f == 0 {
			cur = flipH(cur)
		}
	}
	return result
}

// Placement: list of cell indices (flat) in W*H grid, sorted ascending
type Placement struct {
	cells    []int
	minCell  int // cells[0] after sorting
}

// Compute all valid placements for a given orientation of a shape in a W x H grid
// Returns placements sorted by minCell
func computePlacements(shape Shape, W, H int) []Placement {
	var result []Placement
	for dr := 0; dr+shape.rows <= H; dr++ {
		for dc := 0; dc+shape.cols <= W; dc++ {
			cells := make([]int, len(shape.cells))
			for i, c := range shape.cells {
				r := c[0] + dr
				col := c[1] + dc
				cells[i] = r*W + col
			}
			// sort cells
			sortInts(cells)
			result = append(result, Placement{cells: cells, minCell: cells[0]})
		}
	}
	return result
}

func sortInts(a []int) {
	n := len(a)
	for i := 0; i < n; i++ {
		for j := i + 1; j < n; j++ {
			if a[j] < a[i] {
				a[i], a[j] = a[j], a[i]
			}
		}
	}
}

// Region spec
type Region struct {
	W, H   int
	counts []int
}

func parseRegions(lines []string, numShapes int) []Region {
	var regions []Region
	for _, line := range lines {
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}
		colonIdx := strings.Index(line, ":")
		if colonIdx < 0 {
			continue
		}
		dimPart := strings.TrimSpace(line[:colonIdx])
		countPart := strings.TrimSpace(line[colonIdx+1:])
		if !strings.Contains(dimPart, "x") {
			continue
		}
		parts := strings.Split(dimPart, "x")
		if len(parts) != 2 {
			continue
		}
		W, err1 := strconv.Atoi(strings.TrimSpace(parts[0]))
		H, err2 := strconv.Atoi(strings.TrimSpace(parts[1]))
		if err1 != nil || err2 != nil {
			continue
		}
		cstrs := strings.Fields(countPart)
		counts := make([]int, numShapes)
		for i, cs := range cstrs {
			if i >= numShapes {
				break
			}
			v, err := strconv.Atoi(cs)
			if err == nil {
				counts[i] = v
			}
		}
		regions = append(regions, Region{W: W, H: H, counts: counts})
	}
	return regions
}

// canFit: can we fit all pieces described by counts into a W x H grid?
// allTypePlacements[i] = all valid placements for shape type i (sorted by minCell)
func canFit(W, H int, counts []int, allTypePlacements [][]Placement) bool {
	total := 0
	for _, c := range counts {
		total += c
	}
	if total == 0 {
		return true
	}

	// Build ordered list of (type, count) groups
	// We'll backtrack over types in order, placing all pieces of type 0, then type 1, etc.
	// Within same type, enforce ordering: each new piece must have minCell > previous piece's minCell
	// (to avoid counting same arrangement multiple times)

	// Flatten into groups: [(typeIdx, remaining)]
	type Group struct {
		typeIdx   int
		remaining int
	}
	var groups []Group
	for i, c := range counts {
		if c > 0 {
			groups = append(groups, Group{i, c})
		}
	}

	grid := make([]bool, W*H)
	return backtrack2(grid, groups, 0, 0, allTypePlacements)
}

// backtrack2: place pieces group by group
// groups[gi] is the current group we're placing pieces for
// placed = how many pieces of groups[gi] have been placed so far
// lastMinCell = minCell of the last placed piece in current group (for ordering constraint)
func backtrack2(grid []bool, groups []Group, gi, placed, lastMinCell int, allTypePlacements [][]Placement) bool {
	if gi == len(groups) {
		return true
	}
	g := groups[gi]
	if placed == g.remaining {
		// Done with this group, move to next
		return backtrack2(grid, groups, gi+1, 0, 0, allTypePlacements)
	}

	// Place the next piece of type g.typeIdx
	// Enforce: minCell > lastMinCell (to avoid duplicate orderings within same type)
	placements := allTypePlacements[g.typeIdx]
	for _, p := range placements {
		if p.minCell <= lastMinCell && placed > 0 {
			continue
		}
		// Check if p can be placed
		if !canPlace(grid, p) {
			continue
		}
		place(grid, p)
		if backtrack2(grid, groups, gi, placed+1, p.minCell, allTypePlacements) {
			unplace(grid, p)
			return true
		}
		unplace(grid, p)
	}
	return false
}

func canPlace(grid []bool, p Placement) bool {
	for _, idx := range p.cells {
		if grid[idx] {
			return false
		}
	}
	return true
}

func place(grid []bool, p Placement) {
	for _, idx := range p.cells {
		grid[idx] = true
	}
}

func unplace(grid []bool, p Placement) {
	for _, idx := range p.cells {
		grid[idx] = false
	}
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

	f, err := os.Open("input.txt")
	if err != nil {
		fmt.Fprintln(os.Stderr, "cannot open input.txt:", err)
		os.Exit(1)
	}
	defer f.Close()

	var lines []string
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}

	// Find where regions start
	firstRegion := -1
	for i, line := range lines {
		l := strings.TrimSpace(line)
		if strings.Contains(l, "x") && strings.Contains(l, ":") {
			colonIdx := strings.Index(l, ":")
			dimPart := l[:colonIdx]
			if strings.Contains(dimPart, "x") {
				parts := strings.Split(dimPart, "x")
				if len(parts) == 2 {
					_, e1 := strconv.Atoi(strings.TrimSpace(parts[0]))
					_, e2 := strconv.Atoi(strings.TrimSpace(parts[1]))
					if e1 == nil && e2 == nil {
						firstRegion = i
						break
					}
				}
			}
		}
	}

	if firstRegion < 0 {
		fmt.Fprintln(os.Stderr, "No regions found")
		os.Exit(1)
	}

	shapesLines := lines[:firstRegion]
	regionsLines := lines[firstRegion:]

	shapes := parseShapes(shapesLines)
	numShapes := len(shapes)
	regions := parseRegions(regionsLines, numShapes)

	fmt.Fprintf(os.Stderr, "Parsed %d shapes and %d regions\n", numShapes, len(regions))

	// For each shape, compute all unique orientations
	shapeOrientations := make([][]Shape, numShapes)
	for i, s := range shapes {
		shapeOrientations[i] = allOrientations(s)
	}

	if part == 0 || part == 1 {
		count := 0
		for ri, region := range regions {
			W, H := region.W, region.H
			allTypePlacements := make([][]Placement, numShapes)
			for i := 0; i < numShapes; i++ {
				if region.counts[i] == 0 {
					continue
				}
				for _, orient := range shapeOrientations[i] {
					placements := computePlacements(orient, W, H)
					allTypePlacements[i] = append(allTypePlacements[i], placements...)
				}
				// Sort placements by minCell
				sortPlacements(allTypePlacements[i])
			}

			ok := canFit(W, H, region.counts, allTypePlacements)
			if ok {
				count++
			}
			fmt.Fprintf(os.Stderr, "Region %d (%dx%d counts=%v): %v\n", ri, W, H, region.counts, ok)
		}
		fmt.Printf("Part 1: %d\n", count)
	}

	if part == 2 {
		fmt.Printf("Part 2: (not implemented)\n")
	}
}

func sortPlacements(ps []Placement) {
	// Sort by minCell
	n := len(ps)
	for i := 0; i < n; i++ {
		for j := i + 1; j < n; j++ {
			if ps[j].minCell < ps[i].minCell {
				ps[i], ps[j] = ps[j], ps[i]
			}
		}
	}
}
