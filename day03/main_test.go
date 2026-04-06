package main

import (
	"os"
	"testing"
)

func TestPart1Example(t *testing.T) {
	data, err := os.ReadFile("example.txt")
	if err != nil {
		t.Fatal(err)
	}
	got := solve(string(data))
	want := int64(357)
	if got != want {
		t.Errorf("Part 1 = %d, want %d", got, want)
	}
}

func TestPart2Example(t *testing.T) {
	data, err := os.ReadFile("example.txt")
	if err != nil {
		t.Fatal(err)
	}
	got := solve2(string(data))
	want := int64(3121910778619)
	if got != want {
		t.Errorf("Part 2 = %d, want %d", got, want)
	}
}
