package main

import (
	"fmt"
	"os"
	"sort"
	"strconv"
	"strings"
)

type Range struct {
	lo, hi int64
}

func parseInput(filename string) []Range {
	data, err := os.ReadFile(filename)
	if err != nil {
		panic(err)
	}
	// Remove all whitespace/newlines and split by comma
	s := strings.ReplaceAll(string(data), "\n", "")
	s = strings.ReplaceAll(s, "\r", "")
	s = strings.TrimSpace(s)
	parts := strings.Split(s, ",")
	var ranges []Range
	for _, p := range parts {
		p = strings.TrimSpace(p)
		if p == "" {
			continue
		}
		dash := strings.Index(p, "-")
		if dash < 0 {
			continue
		}
		lo, _ := strconv.ParseInt(p[:dash], 10, 64)
		hi, _ := strconv.ParseInt(p[dash+1:], 10, 64)
		ranges = append(ranges, Range{lo, hi})
	}
	return ranges
}

// inRanges checks if n is in any of the ranges (ranges must be sorted by lo)
func inRanges(n int64, ranges []Range) bool {
	// Binary search: find rightmost range with lo <= n
	lo, hi := 0, len(ranges)-1
	for lo <= hi {
		mid := (lo + hi) / 2
		if ranges[mid].lo <= n {
			lo = mid + 1
		} else {
			hi = mid - 1
		}
	}
	if hi < 0 {
		return false
	}
	return n <= ranges[hi].hi
}

// numDigits returns the number of decimal digits in n (n > 0)
func numDigits(n int64) int {
	d := 0
	for n > 0 {
		d++
		n /= 10
	}
	return d
}

// pow10 returns 10^n as int64
func pow10(n int) int64 {
	result := int64(1)
	for i := 0; i < n; i++ {
		result *= 10
	}
	return result
}

// generatePart1Candidates returns all numbers <= maxVal whose decimal representation
// is some digit-sequence repeated exactly twice (no leading zeros in either half).
// E.g.: 11, 22, ..., 99, 1010, 1111, ..., 9999, 100100, ...
func generatePart1Candidates(maxVal int64) []int64 {
	maxDigits := numDigits(maxVal)
	var result []int64
	for totalLen := 2; totalLen <= maxDigits; totalLen += 2 {
		half := totalLen / 2
		patMin := pow10(half - 1)
		patMax := pow10(half) - 1
		multiplier := pow10(half) + 1 // e.g. half=2 -> 101, number = pat * 101
		for pat := patMin; pat <= patMax; pat++ {
			candidate := pat * multiplier
			if candidate > maxVal {
				break
			}
			result = append(result, candidate)
		}
	}
	return result
}

// generatePart2Candidates returns all numbers <= maxVal whose decimal representation
// is some digit-sequence repeated at least twice (no leading zeros in the pattern).
// E.g.: 11, 111, 1111, ..., 1212, 123123, ...
func generatePart2Candidates(maxVal int64) []int64 {
	maxDigits := numDigits(maxVal)
	seen := make(map[int64]bool)
	for totalLen := 2; totalLen <= maxDigits; totalLen++ {
		for patLen := 1; patLen < totalLen; patLen++ {
			if totalLen%patLen != 0 {
				continue
			}
			repeat := totalLen / patLen
			if repeat < 2 {
				continue
			}
			// multiplier = 1 + 10^patLen + 10^(2*patLen) + ... + 10^((repeat-1)*patLen)
			pow10pat := pow10(patLen)
			multiplier := int64(0)
			pw := int64(1)
			for i := 0; i < repeat; i++ {
				multiplier += pw
				pw *= pow10pat
			}
			patMin := pow10(patLen - 1)
			patMax := pow10(patLen) - 1
			for pat := patMin; pat <= patMax; pat++ {
				candidate := pat * multiplier
				if candidate > maxVal {
					break
				}
				seen[candidate] = true
			}
		}
	}
	result := make([]int64, 0, len(seen))
	for k := range seen {
		result = append(result, k)
	}
	return result
}

func solve(filename string, part int) int64 {
	ranges := parseInput(filename)
	// Sort ranges by lo for binary search
	sort.Slice(ranges, func(i, j int) bool {
		return ranges[i].lo < ranges[j].lo
	})

	// Find max value across all ranges
	maxVal := int64(0)
	for _, r := range ranges {
		if r.hi > maxVal {
			maxVal = r.hi
		}
	}

	var candidates []int64
	if part == 1 {
		candidates = generatePart1Candidates(maxVal)
	} else {
		candidates = generatePart2Candidates(maxVal)
	}

	total := int64(0)
	for _, c := range candidates {
		if inRanges(c, ranges) {
			total += c
		}
	}
	return total
}

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

	if part == 0 || part == 1 {
		fmt.Printf("Part 1: %d\n", solve("input.txt", 1))
	}
	if part == 0 || part == 2 {
		fmt.Printf("Part 2: %d\n", solve("input.txt", 2))
	}
}
