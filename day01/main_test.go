package main

import (
	"testing"
)

func TestPart1(t *testing.T) {
	got, _ := solve("example.txt")
	want := 3
	if got != want {
		t.Errorf("Part 1 = %d, want %d", got, want)
	}
}

func TestPart2(t *testing.T) {
	_, got := solve("example.txt")
	want := 6
	if got != want {
		t.Errorf("Part 2 = %d, want %d", got, want)
	}
}
