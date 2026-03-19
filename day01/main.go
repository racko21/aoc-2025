// Day 1: Secret Entrance
// Simulate a circular dial (0–99) starting at 50.
// Part 1: count rotations that end at 0.
// Part 2: count every click through 0 (including mid-rotation passes).
package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
)

// countZeroCrossings counts how many times position 0 is hit when rotating
// dist steps from pos (L = decreasing, R = increasing).
func countZeroCrossings(pos, dist int, dir byte) int {
	var firstHit int
	if dir == 'L' {
		if pos == 0 {
			firstHit = 100
		} else {
			firstHit = pos
		}
	} else {
		if pos == 0 {
			firstHit = 100
		} else {
			firstHit = 100 - pos
		}
	}
	if dist < firstHit {
		return 0
	}
	return (dist-firstHit)/100 + 1
}

func solve(filename string) (int, int) {
	f, err := os.Open(filename)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()

	pos := 50
	part1, part2 := 0, 0

	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := scanner.Text()
		if len(line) < 2 {
			continue
		}
		dir := line[0]
		dist, err := strconv.Atoi(line[1:])
		if err != nil {
			log.Fatal(err)
		}
		part2 += countZeroCrossings(pos, dist, dir)
		if dir == 'L' {
			pos = ((pos-dist)%100 + 100) % 100
		} else {
			pos = (pos + dist) % 100
		}
		if pos == 0 {
			part1++
		}
	}
	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}

	return part1, part2
}

func main() {
	part1, part2 := solve("input.txt")
	fmt.Println("Part 1:", part1)
	fmt.Println("Part 2:", part2)
}
