// Day 3: Lobby
// Each line of digits is a battery bank. Part 1: select exactly 2 digits in order
// to form the largest possible 2-digit number. Part 2: select exactly 12 digits.
// Greedy: at each pick, choose the max digit in the valid window, leaving enough
// digits remaining for the rest of the picks.
package main

import (
	"fmt"
	"log"
	"os"
	"strings"
	"time"
)

// maxKJoltage returns the largest k-digit number that can be formed by choosing
// k digits from s in their original order, using a greedy window approach.
func maxKJoltage(s string, k int) int64 {
	n := len(s)
	prev := -1
	var result int64
	for i := 0; i < k; i++ {
		// Window: [prev+1, n-k+i] — must leave k-i-1 digits after this pick.
		lo := prev + 1
		hi := n - k + i
		best := byte('0')
		bestPos := lo
		for p := lo; p <= hi; p++ {
			if s[p] > best {
				best = s[p]
				bestPos = p
			}
		}
		result = result*10 + int64(best-'0')
		prev = bestPos
	}
	return result
}

func solve(input string) int64 {
	var total int64
	for _, line := range strings.Split(strings.TrimSpace(input), "\n") {
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}
		total += maxKJoltage(line, 2)
	}
	return total
}

func solve2(input string) int64 {
	var total int64
	for _, line := range strings.Split(strings.TrimSpace(input), "\n") {
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}
		total += maxKJoltage(line, 12)
	}
	return total
}

func main() {
	if len(os.Args) > 1 {
		switch os.Args[1] {
		case "1":
			start := time.Now()
			data, err := os.ReadFile("input.txt")
			if err != nil {
				log.Fatal(err)
			}
			result := solve(string(data))
			fmt.Fprintf(os.Stderr, "runtime_ms: %.2f\n", float64(time.Since(start).Microseconds())/1000)
			fmt.Println("Part 1:", result)
			return
		case "2":
			start := time.Now()
			data, err := os.ReadFile("input.txt")
			if err != nil {
				log.Fatal(err)
			}
			result := solve2(string(data))
			fmt.Fprintf(os.Stderr, "runtime_ms: %.2f\n", float64(time.Since(start).Microseconds())/1000)
			fmt.Println("Part 2:", result)
			return
		}
	}
	data, err := os.ReadFile("input.txt")
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println("Part 1:", solve(string(data)))
	fmt.Println("Part 2:", solve2(string(data)))
}
