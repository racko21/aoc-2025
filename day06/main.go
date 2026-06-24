// Day 6: parse columnar math worksheet; sum results of each +/* problem column.
// Part 1: each row in a band is one number. Part 2: each column in a band is one number (right-to-left).
package main

import (
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/racko21/aoc-2025/utils"
)

func loadGrid(path string) ([][]byte, int) {
	lines := utils.ReadLines(path)
	for len(lines) > 0 && strings.TrimSpace(lines[len(lines)-1]) == "" {
		lines = lines[:len(lines)-1]
	}
	maxW := 0
	for _, l := range lines {
		if len(l) > maxW {
			maxW = len(l)
		}
	}
	padded := make([][]byte, len(lines))
	for i, l := range lines {
		b := make([]byte, maxW)
		copy(b, l)
		for j := len(l); j < maxW; j++ {
			b[j] = ' '
		}
		padded[i] = b
	}
	return padded, maxW
}

func separators(padded [][]byte, maxW int) []bool {
	sep := make([]bool, maxW)
	for c := 0; c < maxW; c++ {
		all := true
		for _, row := range padded {
			if row[c] != ' ' {
				all = false
				break
			}
		}
		sep[c] = all
	}
	return sep
}

func applyOp(op byte, nums []int64) int64 {
	if op == '+' {
		var s int64
		for _, n := range nums {
			s += n
		}
		return s
	}
	p := int64(1)
	for _, n := range nums {
		p *= n
	}
	return p
}

// solvePart1Band: each number row in the band is one number.
func solvePart1Band(padded [][]byte, lastRow, lo, hi int) int64 {
	opStr := strings.TrimSpace(string(padded[lastRow][lo:hi]))
	if len(opStr) == 0 {
		return 0
	}
	op := opStr[0]
	var nums []int64
	for r := 0; r < lastRow; r++ {
		s := strings.TrimSpace(string(padded[r][lo:hi]))
		if s != "" {
			nums = append(nums, utils.MustParseInt64(s))
		}
	}
	return applyOp(op, nums)
}

// solvePart2Band: each column (right-to-left) in the band is one number (digits top-to-bottom).
func solvePart2Band(padded [][]byte, lastRow, lo, hi int) int64 {
	opStr := strings.TrimSpace(string(padded[lastRow][lo:hi]))
	if len(opStr) == 0 {
		return 0
	}
	op := opStr[0]
	var nums []int64
	for c := hi - 1; c >= lo; c-- {
		var sb strings.Builder
		for r := 0; r < lastRow; r++ {
			if padded[r][c] != ' ' {
				sb.WriteByte(padded[r][c])
			}
		}
		if s := sb.String(); s != "" {
			nums = append(nums, utils.MustParseInt64(s))
		}
	}
	return applyOp(op, nums)
}

func solve(path string, bandSolver func([][]byte, int, int, int) int64) int64 {
	padded, maxW := loadGrid(path)
	if len(padded) == 0 {
		return 0
	}
	sep := separators(padded, maxW)
	lastRow := len(padded) - 1
	var total int64
	inBand := false
	start := 0
	for c := 0; c <= maxW; c++ {
		if c == maxW || sep[c] {
			if inBand {
				total += bandSolver(padded, lastRow, start, c)
				inBand = false
			}
		} else if !inBand {
			start = c
			inBand = true
		}
	}
	return total
}

func part1(path string) int64 { return solve(path, solvePart1Band) }
func part2(path string) int64 { return solve(path, solvePart2Band) }

func main() {
	if len(os.Args) > 1 {
		switch os.Args[1] {
		case "1":
			start := time.Now()
			result := part1("input.txt")
			fmt.Fprintf(os.Stderr, "runtime_ms: %.2f\n", float64(time.Since(start).Microseconds())/1000)
			fmt.Println("Part 1:", result)
			return
		case "2":
			start := time.Now()
			result := part2("input.txt")
			fmt.Fprintf(os.Stderr, "runtime_ms: %.2f\n", float64(time.Since(start).Microseconds())/1000)
			fmt.Println("Part 2:", result)
			return
		}
	}
	fmt.Println("Part 1:", part1("input.txt"))
	fmt.Println("Part 2:", part2("input.txt"))
}
