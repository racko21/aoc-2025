package main

import (
	"testing"

	utils "github.com/racko21/aoc-2025/utils"
)

func TestPart1(t *testing.T) {
	lines := utils.ReadLines("example.txt")
	shapeOrientations, shapeCells := parseShapes(lines)
	regions := parseRegions(lines, len(shapeOrientations))

	count := 0
	for _, reg := range regions {
		if canFit(reg.W, reg.H, shapeOrientations, shapeCells, reg.counts) {
			count++
		}
	}
	if count != 2 {
		t.Errorf("Part 1: got %d, want 2", count)
	}
}
