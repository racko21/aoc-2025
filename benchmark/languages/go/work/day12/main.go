package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

type shape struct {
	cells   int
	orients [][][2]int
}

func parseShape(rows []string) shape {
	var base [][2]int
	for r, line := range rows {
		for c, ch := range line {
			if ch == '#' {
				base = append(base, [2]int{r, c})
			}
		}
	}
	s := shape{cells: len(base)}
	seen := map[string]bool{}
	cur := base
	for flip := 0; flip < 2; flip++ {
		c := cur
		for rot := 0; rot < 4; rot++ {
			norm, key := normalize(c)
			if !seen[key] {
				seen[key] = true
				s.orients = append(s.orients, norm)
			}
			c = rotate(c)
		}
		cur = flipCells(cur)
	}
	return s
}

func rotate(cells [][2]int) [][2]int {
	out := make([][2]int, len(cells))
	for i, p := range cells {
		out[i] = [2]int{p[1], -p[0]}
	}
	return out
}
func flipCells(cells [][2]int) [][2]int {
	out := make([][2]int, len(cells))
	for i, p := range cells {
		out[i] = [2]int{p[0], -p[1]}
	}
	return out
}
func normalize(cells [][2]int) ([][2]int, string) {
	minr, minc := 1<<30, 1<<30
	for _, p := range cells {
		if p[0] < minr {
			minr = p[0]
		}
		if p[1] < minc {
			minc = p[1]
		}
	}
	out := make([][2]int, len(cells))
	for i, p := range cells {
		out[i] = [2]int{p[0] - minr, p[1] - minc}
	}
	for i := 0; i < len(out); i++ {
		for j := i + 1; j < len(out); j++ {
			if out[j][0] < out[i][0] || (out[j][0] == out[i][0] && out[j][1] < out[i][1]) {
				out[i], out[j] = out[j], out[i]
			}
		}
	}
	var sb strings.Builder
	for _, p := range out {
		sb.WriteString(strconv.Itoa(p[0]))
		sb.WriteByte(',')
		sb.WriteString(strconv.Itoa(p[1]))
		sb.WriteByte(';')
	}
	return out, sb.String()
}

type region struct {
	w, h   int
	counts []int
}

func main() {
	part := 0
	if len(os.Args) > 1 {
		part, _ = strconv.Atoi(os.Args[1])
	}
	data, _ := os.ReadFile("input.txt")
	lines := strings.Split(string(data), "\n")
	var shapes []shape
	var regions []region
	i := 0
	for i < len(lines) {
		t := strings.TrimSpace(strings.TrimRight(lines[i], "\r"))
		if t == "" {
			i++
			continue
		}
		if strings.HasSuffix(t, ":") && !strings.Contains(t, "x") {
			var rows []string
			i++
			for i < len(lines) {
				rt := strings.TrimSpace(strings.TrimRight(lines[i], "\r"))
				if rt == "" {
					break
				}
				if strings.Contains(rt, "x") && strings.Contains(rt, ":") {
					break
				}
				rows = append(rows, rt)
				i++
			}
			shapes = append(shapes, parseShape(rows))
			continue
		}
		if strings.Contains(t, "x") && strings.Contains(t, ":") {
			parts := strings.SplitN(t, ":", 2)
			dims := strings.Split(strings.TrimSpace(parts[0]), "x")
			w, _ := strconv.Atoi(dims[0])
			h, _ := strconv.Atoi(dims[1])
			fields := strings.Fields(strings.TrimSpace(parts[1]))
			cnts := make([]int, len(fields))
			for j, f := range fields {
				cnts[j], _ = strconv.Atoi(f)
			}
			regions = append(regions, region{w, h, cnts})
			i++
			continue
		}
		i++
	}

	fitCount := 0
	for _, rg := range regions {
		if fits(rg, shapes) {
			fitCount++
		}
	}

	w := bufio.NewWriter(os.Stdout)
	defer w.Flush()
	if part == 1 || part == 0 {
		fmt.Fprintf(w, "Part 1: %d\n", fitCount)
	}
}

func fits(rg region, shapes []shape) bool {
	used, total := 0, 0
	for idx, c := range rg.counts {
		used += c * shapes[idx].cells
		total += c
	}
	if used > rg.w*rg.h {
		return false
	}
	// sufficient: each piece in its own disjoint 3x3 box
	boxes := (rg.w / 3) * (rg.h / 3)
	if total <= boxes {
		return true
	}
	// tight case: real backtracking packing (allowing empty cells)
	return packBacktrack(rg, shapes)
}

func packBacktrack(rg region, shapes []shape) bool {
	W, H := rg.w, rg.h
	N := W * H
	grid := make([]bool, N)
	rem := make([]int, len(shapes))
	copy(rem, rg.counts)
	total := 0
	for _, c := range rem {
		total += c
	}
	freeCells := N // currently free cells
	var solve func(placed, startPos, free int) bool
	solve = func(placed, startPos, free int) bool {
		if placed == total {
			return true
		}
		// remaining pieces need at least this many cells
		need := 0
		for si, c := range rem {
			need += c * shapes[si].cells
		}
		if need > free {
			return false
		}
		// find next free cell at or after startPos
		pos := -1
		for p := startPos; p < N; p++ {
			if !grid[p] {
				pos = p
				break
			}
		}
		if pos < 0 {
			return false
		}
		pr, pc := pos/W, pos%W
		// Option A: place a piece covering pos (anchor each cell on pos)
		for si := range shapes {
			if rem[si] == 0 {
				continue
			}
			sh := shapes[si]
			for _, ori := range sh.orients {
				for _, anchor := range ori {
					or0, oc0 := anchor[0], anchor[1]
					ok := true
					for _, cell := range ori {
						rr := pr + (cell[0] - or0)
						cc := pc + (cell[1] - oc0)
						if rr < 0 || rr >= H || cc < 0 || cc >= W || grid[rr*W+cc] {
							ok = false
							break
						}
						if rr*W+cc < pos {
							ok = false
							break
						}
					}
					if !ok {
						continue
					}
					for _, cell := range ori {
						rr := pr + (cell[0] - or0)
						cc := pc + (cell[1] - oc0)
						grid[rr*W+cc] = true
					}
					rem[si]--
					if solve(placed+1, pos, free-sh.cells) {
						return true
					}
					rem[si]++
					for _, cell := range ori {
						rr := pr + (cell[0] - or0)
						cc := pc + (cell[1] - oc0)
						grid[rr*W+cc] = false
					}
				}
			}
		}
		// Option B: leave pos empty, move on
		if solve(placed, pos+1, free-1) {
			return true
		}
		return false
	}
	return solve(0, 0, freeCells)
}
