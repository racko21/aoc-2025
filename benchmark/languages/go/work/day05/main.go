package main

import (
	"fmt"
	"os"
	"sort"
	"strconv"
	"strings"
)

func main() {
	data, _ := os.ReadFile("input.txt")
	parts := strings.SplitN(strings.ReplaceAll(string(data), "\r\n", "\n"), "\n\n", 2)
	type rng struct{ lo, hi int }
	var ranges []rng
	for _, line := range strings.Split(strings.TrimSpace(parts[0]), "\n") {
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}
		ab := strings.Split(line, "-")
		lo, _ := strconv.Atoi(strings.TrimSpace(ab[0]))
		hi, _ := strconv.Atoi(strings.TrimSpace(ab[1]))
		ranges = append(ranges, rng{lo, hi})
	}

	part := 0
	if len(os.Args) > 1 {
		part, _ = strconv.Atoi(os.Args[1])
	}

	if part == 0 || part == 1 {
		count := 0
		if len(parts) > 1 {
			for _, line := range strings.Split(strings.TrimSpace(parts[1]), "\n") {
				line = strings.TrimSpace(line)
				if line == "" {
					continue
				}
				id, _ := strconv.Atoi(line)
				for _, r := range ranges {
					if id >= r.lo && id <= r.hi {
						count++
						break
					}
				}
			}
		}
		fmt.Printf("Part 1: %d\n", count)
	}

	if part == 0 || part == 2 {
		sort.Slice(ranges, func(i, j int) bool { return ranges[i].lo < ranges[j].lo })
		total := 0
		curLo, curHi := -1, -1
		for _, r := range ranges {
			if curHi == -1 {
				curLo, curHi = r.lo, r.hi
				continue
			}
			if r.lo <= curHi+1 {
				if r.hi > curHi {
					curHi = r.hi
				}
			} else {
				total += curHi - curLo + 1
				curLo, curHi = r.lo, r.hi
			}
		}
		if curHi != -1 {
			total += curHi - curLo + 1
		}
		fmt.Printf("Part 2: %d\n", total)
	}
}
