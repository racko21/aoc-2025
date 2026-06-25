package main

import (
	"bufio"
	"fmt"
	"os"
	"sort"
	"strconv"
	"strings"
)

type Range struct {
	lo, hi int
}

func parseRange(line string) (Range, bool) {
	var lo, hi int
	// Try format "lo-hi"
	n, err := fmt.Sscanf(line, "%d-%d", &lo, &hi)
	if err == nil && n == 2 {
		if lo > hi {
			lo, hi = hi, lo
		}
		return Range{lo, hi}, true
	}
	return Range{}, false
}

func parseInt(line string) (int, bool) {
	n, err := strconv.Atoi(strings.TrimSpace(line))
	if err != nil {
		return 0, false
	}
	return n, true
}

func parseInput(filename string) ([]Range, []Range, error) {
	f, err := os.Open(filename)
	if err != nil {
		return nil, nil, err
	}
	defer f.Close()

	var freshRanges []Range
	// Second section: either individual IDs (stored as single-element ranges) or ranges
	var availRanges []Range
	inSecond := false

	scanner := bufio.NewScanner(f)
	scanner.Buffer(make([]byte, 1024*1024), 1024*1024)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			inSecond = true
			continue
		}

		if !inSecond {
			r, ok := parseRange(line)
			if !ok {
				return nil, nil, fmt.Errorf("bad fresh range: %q", line)
			}
			freshRanges = append(freshRanges, r)
		} else {
			// Try as range first
			if r, ok := parseRange(line); ok {
				availRanges = append(availRanges, r)
			} else if n, ok := parseInt(line); ok {
				// Single integer
				availRanges = append(availRanges, Range{n, n})
			} else {
				return nil, nil, fmt.Errorf("bad available item: %q", line)
			}
		}
	}

	return freshRanges, availRanges, scanner.Err()
}

// mergeRanges merges overlapping ranges and returns the merged list sorted by lo.
func mergeRanges(ranges []Range) []Range {
	if len(ranges) == 0 {
		return nil
	}
	sorted := make([]Range, len(ranges))
	copy(sorted, ranges)
	sort.Slice(sorted, func(i, j int) bool {
		if sorted[i].lo != sorted[j].lo {
			return sorted[i].lo < sorted[j].lo
		}
		return sorted[i].hi < sorted[j].hi
	})

	merged := []Range{sorted[0]}
	for _, r := range sorted[1:] {
		last := &merged[len(merged)-1]
		if r.lo <= last.hi+1 {
			if r.hi > last.hi {
				last.hi = r.hi
			}
		} else {
			merged = append(merged, r)
		}
	}
	return merged
}

// isFreshBinary uses binary search on merged (sorted) ranges
func isFreshBinary(id int, merged []Range) bool {
	lo, hi := 0, len(merged)-1
	for lo <= hi {
		mid := (lo + hi) / 2
		r := merged[mid]
		if id < r.lo {
			hi = mid - 1
		} else if id > r.hi {
			lo = mid + 1
		} else {
			return true
		}
	}
	return false
}

// countFreshInRange counts how many integers in [qlo, qhi] are in merged fresh ranges
func countFreshInRange(qlo, qhi int, merged []Range) int {
	// Find all merged ranges that overlap with [qlo, qhi] and sum
	count := 0
	for _, r := range merged {
		if r.hi < qlo || r.lo > qhi {
			continue
		}
		lo := r.lo
		if lo < qlo {
			lo = qlo
		}
		hi := r.hi
		if hi > qhi {
			hi = qhi
		}
		count += hi - lo + 1
	}
	return count
}

func part1(freshRanges []Range, availRanges []Range) int {
	merged := mergeRanges(freshRanges)

	// Check each available item
	// If single ID (lo==hi), check if fresh
	// If range, count how many in that range are fresh
	count := 0
	for _, ar := range availRanges {
		if ar.lo == ar.hi {
			// Single ID
			if isFreshBinary(ar.lo, merged) {
				count++
			}
		} else {
			// Range of IDs — count fresh ones
			count += countFreshInRange(ar.lo, ar.hi, merged)
		}
	}
	return count
}

func part2(freshRanges []Range) int {
	merged := mergeRanges(freshRanges)
	total := 0
	for _, r := range merged {
		total += r.hi - r.lo + 1
	}
	return total
}

func main() {
	freshRanges, availRanges, err := parseInput("input.txt")
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %v\n", err)
		os.Exit(1)
	}

	part := ""
	if len(os.Args) > 1 {
		part = os.Args[1]
	}

	switch part {
	case "1":
		fmt.Printf("Part 1: %d\n", part1(freshRanges, availRanges))
	case "2":
		fmt.Printf("Part 2: %d\n", part2(freshRanges))
	default:
		fmt.Printf("Part 1: %d\n", part1(freshRanges, availRanges))
		fmt.Printf("Part 2: %d\n", part2(freshRanges))
	}
}
