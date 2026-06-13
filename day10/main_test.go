package main

import (
	"testing"
)

func TestPart1(t *testing.T) {
	got := part1("example.txt")
	want := 7
	if got != want {
		t.Errorf("Part1 = %d, want %d", got, want)
	}
}

func TestPart2(t *testing.T) {
	got := part2("example.txt")
	want := int64(33)
	if got != want {
		t.Errorf("Part2 = %d, want %d", got, want)
	}
}
