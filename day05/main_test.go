package main

import (
	"testing"
)

func TestPart1(t *testing.T) {
	ranges, ids := parse("example.txt")
	got := part1(ranges, ids)
	want := 3
	if got != want {
		t.Errorf("Part1 = %d, want %d", got, want)
	}
}

func TestPart2(t *testing.T) {
	ranges, _ := parse("example.txt")
	got := part2(ranges)
	want := 14
	if got != want {
		t.Errorf("Part2 = %d, want %d", got, want)
	}
}
