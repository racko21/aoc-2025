package main

import (
	"testing"
)

func TestPart1(t *testing.T) {
	grid := parseGrid("example.txt")
	got := part1(grid)
	want := 13
	if got != want {
		t.Errorf("Part1 = %d, want %d", got, want)
	}
}

func TestPart2(t *testing.T) {
	grid := parseGrid("example.txt")
	got := part2(grid)
	want := 43
	if got != want {
		t.Errorf("Part2 = %d, want %d", got, want)
	}
}
