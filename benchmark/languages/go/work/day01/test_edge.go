package main

import "fmt"

func countZerosDuring(pos, dist int, dir string) int {
	var firstI int
	if dir == "L" {
		if pos == 0 {
			firstI = 100
		} else {
			firstI = pos
		}
	} else {
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

func main() {
	// R1000 from 50 should hit 0 ten times
	fmt.Println("R1000 from 50:", countZerosDuring(50, 1000, "R"))
	// final pos = (50 + 1000) % 100 = 1050 % 100 = 50, so it returns to 50 hitting 0 ten times
	
	// Verify: first hit at i=50, then 150, 250, 350, 450, 550, 650, 750, 850, 950
	// That's 10 hits. (1000-50)/100 + 1 = 950/100 + 1 = 9 + 1 = 10 ✓
}
