package main

import "testing"

func TestPart1(t *testing.T) {
	p1, _ := solve("example.txt")
	if p1 != 5 {
		t.Errorf("Part 1: got %d, want 5", p1)
	}
}

func TestPart2(t *testing.T) {
	_, p2 := solve("example2.txt")
	if p2 != 2 {
		t.Errorf("Part 2: got %d, want 2", p2)
	}
}
