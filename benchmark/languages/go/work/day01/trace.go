package main

import "fmt"

func main() {
	rotations := []struct{dir string; dist int}{
		{"L", 68}, {"L", 30}, {"R", 48}, {"L", 5}, {"R", 60},
		{"L", 55}, {"L", 1}, {"L", 99}, {"R", 14}, {"L", 82},
	}
	pos := 50
	for _, r := range rotations {
		if r.dir == "L" {
			pos = ((pos - r.dist) % 100 + 100) % 100
		} else {
			pos = (pos + r.dist) % 100
		}
		fmt.Printf("%s%d -> %d\n", r.dir, r.dist, pos)
	}
}
