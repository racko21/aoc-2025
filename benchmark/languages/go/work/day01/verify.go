package main

import "fmt"

func countZerosBrute(pos, dist int, dir string) int {
	count := 0
	for i := 0; i < dist; i++ {
		if dir == "L" {
			pos = ((pos - 1) % 100 + 100) % 100
		} else {
			pos = (pos + 1) % 100
		}
		if pos == 0 {
			count++
		}
	}
	return count
}

func countZerosFormula(pos, dist int, dir string) int {
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
	// Test all combinations
	mismatches := 0
	for pos := 0; pos < 100; pos++ {
		for dist := 0; dist <= 1000; dist++ {
			for _, dir := range []string{"L", "R"} {
				brute := countZerosBrute(pos, dist, dir)
				formula := countZerosFormula(pos, dist, dir)
				if brute != formula {
					fmt.Printf("MISMATCH pos=%d dist=%d dir=%s brute=%d formula=%d\n", pos, dist, dir, brute, formula)
					mismatches++
					if mismatches > 10 {
						return
					}
				}
			}
		}
	}
	if mismatches == 0 {
		fmt.Println("All match!")
	}
}
