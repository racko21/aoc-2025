package main

import (
	"bufio"
	"fmt"
	"os"
)

// maxKDigits returns the largest number (as string) formed by choosing
// exactly k digits from s in their original order.
// Greedy: for each of the k slots, find the max digit in the valid window,
// pick the leftmost occurrence of that max.
func maxKDigits(s string, k int) string {
	n := len(s)
	if k > n {
		return ""
	}
	result := make([]byte, k)
	start := 0
	for slot := 0; slot < k; slot++ {
		// We must pick from s[start .. n-1-(k-1-slot)]
		// After picking, we need (k-1-slot) more digits from the remaining string.
		end := n - (k - 1 - slot) // exclusive upper bound? Let's think:
		// If we pick at index i, we need (k-1-slot) more digits after i.
		// So i can be at most n-1-(k-1-slot), i.e. i <= n-(k-slot)
		// end (exclusive) = n-(k-slot)+1 = n-k+slot+1
		end = n - k + slot + 1 // exclusive

		// Find max digit in s[start..end-1]
		maxDigit := byte('0')
		for i := start; i < end; i++ {
			if s[i] > maxDigit {
				maxDigit = s[i]
			}
		}
		// Find leftmost occurrence of maxDigit in [start, end)
		for i := start; i < end; i++ {
			if s[i] == maxDigit {
				result[slot] = maxDigit
				start = i + 1
				break
			}
		}
	}
	return string(result)
}

// parseNumber converts a string of digits to int64
func parseNumber(s string) int64 {
	var v int64
	for _, c := range s {
		v = v*10 + int64(c-'0')
	}
	return v
}

func solve(lines []string, k int) int64 {
	var total int64
	for _, line := range lines {
		if len(line) == 0 {
			continue
		}
		best := maxKDigits(line, k)
		total += parseNumber(best)
	}
	return total
}

func main() {
	// Read input
	f, err := os.Open("input.txt")
	if err != nil {
		panic(err)
	}
	defer f.Close()

	var lines []string
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := scanner.Text()
		if line != "" {
			lines = append(lines, line)
		}
	}

	part := ""
	if len(os.Args) > 1 {
		part = os.Args[1]
	}

	if part == "" || part == "1" {
		fmt.Printf("Part 1: %d\n", solve(lines, 2))
	}
	if part == "" || part == "2" {
		fmt.Printf("Part 2: %d\n", solve(lines, 12))
	}
}
