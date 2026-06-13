package main

import (
	"testing"
)

func TestPart1(t *testing.T) {
	got := solve("example.txt", 10)
	want := 40
	if got != want {
		t.Errorf("solve(example,10) = %d, want %d", got, want)
	}
}

func TestPart2(t *testing.T) {
	// last merge: 216,146,977 and 117,168,530 → 216*117 = 25272
	got := part2("example.txt")
	want := 25272
	if got != want {
		t.Errorf("Part2(example) = %d, want %d", got, want)
	}
}
