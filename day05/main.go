// Day 5: count fresh IDs by range membership (Part 1) and interval union size (Part 2).
package main

import (
	"fmt"
	"os"
	"sort"
	"strings"
	"time"

	"github.com/racko21/aoc-2025/utils"
)

type interval struct{ lo, hi int }

func parse(path string) ([]interval, []int) {
	lines := utils.ReadLines(path)
	var ranges []interval
	var ids []int
	inIDs := false
	for _, line := range lines {
		if line == "" {
			inIDs = true
			continue
		}
		if inIDs {
			ids = append(ids, utils.Atoi(line))
		} else {
			parts := strings.SplitN(line, "-", 2)
			ranges = append(ranges, interval{utils.Atoi(parts[0]), utils.Atoi(parts[1])})
		}
	}
	return ranges, ids
}

func isFresh(id int, ranges []interval) bool {
	for _, r := range ranges {
		if id >= r.lo && id <= r.hi {
			return true
		}
	}
	return false
}

func part1(ranges []interval, ids []int) int {
	count := 0
	for _, id := range ids {
		if isFresh(id, ranges) {
			count++
		}
	}
	return count
}

func part2(ranges []interval) int {
	sort.Slice(ranges, func(i, j int) bool { return ranges[i].lo < ranges[j].lo })
	total := 0
	lo, hi := ranges[0].lo, ranges[0].hi
	for _, r := range ranges[1:] {
		if r.lo <= hi+1 {
			if r.hi > hi {
				hi = r.hi
			}
		} else {
			total += hi - lo + 1
			lo, hi = r.lo, r.hi
		}
	}
	total += hi - lo + 1
	return total
}

func main() {
	if len(os.Args) > 1 {
		switch os.Args[1] {
		case "1":
			start := time.Now()
			ranges, ids := parse("input.txt")
			result := part1(ranges, ids)
			fmt.Fprintf(os.Stderr, "runtime_ms: %.2f\n", float64(time.Since(start).Microseconds())/1000)
			fmt.Println("Part 1:", result)
			return
		case "2":
			start := time.Now()
			ranges, _ := parse("input.txt")
			result := part2(ranges)
			fmt.Fprintf(os.Stderr, "runtime_ms: %.2f\n", float64(time.Since(start).Microseconds())/1000)
			fmt.Println("Part 2:", result)
			return
		}
	}
	ranges, ids := parse("input.txt")
	fmt.Println("Part 1:", part1(ranges, ids))
	fmt.Println("Part 2:", part2(ranges))
}
