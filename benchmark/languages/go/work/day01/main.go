package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func main() {
	part := 0
	if len(os.Args) > 1 {
		switch os.Args[1] {
		case "1":
			part = 1
		case "2":
			part = 2
		}
	}

	f, err := os.Open("input.txt")
	if err != nil {
		panic(err)
	}
	defer f.Close()

	type Rotation struct {
		dir string
		dist int
	}

	var rotations []Rotation
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if len(line) == 0 {
			continue
		}
		dir := string(line[0])
		dist, err := strconv.Atoi(line[1:])
		if err != nil {
			panic(err)
		}
		rotations = append(rotations, Rotation{dir, dist})
	}

	// Part 1: count times dial ends at 0 after each rotation
	part1 := func() int {
		pos := 50
		count := 0
		for _, r := range rotations {
			if r.dir == "L" {
				pos = ((pos - r.dist) % 100 + 100) % 100
			} else {
				pos = (pos + r.dist) % 100
			}
			if pos == 0 {
				count++
			}
		}
		return count
	}

	// Part 2: count every click that lands on 0 during any rotation
	// Going Left from pos by d clicks: positions are (pos-1)%100, (pos-2)%100, ..., (pos-d)%100
	// We want count of i in [1..d] where (pos - i) % 100 == 0
	// i.e. (pos - i) ≡ 0 mod 100 => i ≡ pos mod 100
	// So first such i = pos (if pos != 0) or 100 (if pos == 0)
	// Then i = pos, pos+100, pos+200, ... up to d
	// If pos == 0, first i = 100
	// Count = floor((d - first_i) / 100) + 1 if first_i <= d, else 0
	//
	// Going Right from pos by d clicks: positions are (pos+1)%100, ..., (pos+d)%100
	// We want count of i in [1..d] where (pos + i) % 100 == 0
	// (pos + i) ≡ 0 mod 100 => i ≡ -pos ≡ (100 - pos) mod 100
	// first_i = (100 - pos) % 100; if first_i == 0, first_i = 100
	// Count = floor((d - first_i) / 100) + 1 if first_i <= d, else 0

	countZerosDuring := func(pos, dist int, dir string) int {
		var firstI int
		if dir == "L" {
			if pos == 0 {
				firstI = 100
			} else {
				firstI = pos
			}
		} else { // R
			firstI = (100 - pos) % 100
			if firstI == 0 {
				firstI = 100
			}
		}
		if firstI > dist {
			return 0
		}
		return (dist-firstI)/100 + 1
	}

	part2 := func() int {
		pos := 50
		count := 0
		for _, r := range rotations {
			count += countZerosDuring(pos, r.dist, r.dir)
			if r.dir == "L" {
				pos = ((pos - r.dist) % 100 + 100) % 100
			} else {
				pos = (pos + r.dist) % 100
			}
		}
		return count
	}

	if part == 0 || part == 1 {
		fmt.Printf("Part 1: %d\n", part1())
	}
	if part == 0 || part == 2 {
		fmt.Printf("Part 2: %d\n", part2())
	}
}
