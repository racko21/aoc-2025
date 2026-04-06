// Day 2: Gift Shop
// Find all "invalid" IDs in given ranges — an invalid ID is a number whose
// decimal digits are some sequence repeated exactly twice (e.g. 6464, 123123).
// A doubled number of length 2k equals n*(10^k+1) for any k-digit number n.
package main

import (
	"fmt"
	"log"
	"os"
	"strings"

	"github.com/racko21/aoc-2025/utils"
)

// parseRanges reads comma-separated lo-hi pairs from a single (possibly multi-line) string.
func parseRanges(s string) [][2]int64 {
	// Join lines in case input is wrapped
	s = strings.ReplaceAll(s, "\n", "")
	s = strings.TrimSpace(s)

	var ranges [][2]int64
	for _, part := range strings.Split(s, ",") {
		part = strings.TrimSpace(part)
		if part == "" {
			continue
		}
		dash := strings.Index(part, "-")
		lo := utils.MustParseInt64(part[:dash])
		hi := utils.MustParseInt64(part[dash+1:])
		ranges = append(ranges, [2]int64{lo, hi})
	}
	return ranges
}

// inAnyRange returns true if n falls within at least one of the ranges.
func inAnyRange(n int64, ranges [][2]int64) bool {
	for _, r := range ranges {
		if n >= r[0] && n <= r[1] {
			return true
		}
	}
	return false
}

// sumDoubledInRanges sums all doubled numbers (n*(10^k+1)) that fall in any range.
func sumDoubledInRanges(ranges [][2]int64) int64 {
	var maxVal int64
	for _, r := range ranges {
		if r[1] > maxVal {
			maxVal = r[1]
		}
	}

	var total int64
	pow10k := int64(10) // 10^k, starting at k=1
	for {
		multiplier := pow10k + 1         // 10^k + 1
		firstN := pow10k / 10            // smallest k-digit number (10^(k-1))
		if firstN == 0 {
			firstN = 1
		}

		// Smallest doubled number for this digit length
		if firstN*multiplier > maxVal {
			break
		}

		for n := firstN; n < pow10k; n++ {
			doubled := n * multiplier
			if doubled > maxVal {
				break
			}
			if inAnyRange(doubled, ranges) {
				total += doubled
			}
		}

		// Advance to k+1; guard against overflow
		next := pow10k * 10
		if next < pow10k {
			break
		}
		pow10k = next
	}
	return total
}

// sumRepeatedInRanges sums all numbers in any range whose digit string is
// some sequence of length k repeated m≥2 times (no leading zeros).
func sumRepeatedInRanges(ranges [][2]int64) int64 {
	var maxVal int64
	for _, r := range ranges {
		if r[1] > maxVal {
			maxVal = r[1]
		}
	}

	seen := make(map[int64]bool)

	// Outer loop: period length k, pow10k = 10^k
	pow10k := int64(10)
	for pow10k <= maxVal {
		firstPrefix := pow10k / 10
		if firstPrefix == 0 {
			firstPrefix = 1
		}

		// multiplier for m repetitions = 1 + 10^k + 10^(2k) + … + 10^((m-1)k)
		// Start at m=2: multiplier = 1 + 10^k, nextPow = 10^(2k)
		multiplier := pow10k + 1
		nextPow := pow10k * pow10k // 10^(2k)

		for firstPrefix*multiplier <= maxVal {
			for n := firstPrefix; n < pow10k; n++ {
				num := n * multiplier
				if num > maxVal {
					break
				}
				if inAnyRange(num, ranges) {
					seen[num] = true
				}
			}
			// Advance to m+1
			if nextPow > maxVal {
				break
			}
			newMult := multiplier + nextPow
			if newMult < multiplier { // overflow
				break
			}
			multiplier = newMult
			newNext := nextPow * pow10k
			if newNext < nextPow { // overflow
				nextPow = maxVal + 1
			} else {
				nextPow = newNext
			}
		}

		next := pow10k * 10
		if next < pow10k { // overflow
			break
		}
		pow10k = next
	}

	var total int64
	for num := range seen {
		total += num
	}
	return total
}

func main() {
	data, err := os.ReadFile("input.txt")
	if err != nil {
		log.Fatal(err)
	}
	ranges := parseRanges(string(data))
	fmt.Println("Part 1:", sumDoubledInRanges(ranges))
	fmt.Println("Part 2:", sumRepeatedInRanges(ranges))
}
