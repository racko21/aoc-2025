package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func readLines2(filename string) []string {
	f, _ := os.Open(filename)
	defer f.Close()
	var lines []string
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	for len(lines) > 0 && strings.TrimSpace(lines[len(lines)-1]) == "" {
		lines = lines[:len(lines)-1]
	}
	return lines
}

func padLines2(lines []string) []string {
	maxLen := 0
	for _, l := range lines { if len(l) > maxLen { maxLen = len(l) } }
	result := make([]string, len(lines))
	for i, l := range lines {
		if len(l) < maxLen { result[i] = l + strings.Repeat(" ", maxLen-len(l)) } else { result[i] = l }
	}
	return result
}

func isAllSpaces2(lines []string, col int) bool {
	for _, l := range lines { if col < len(l) && l[col] != ' ' { return false } }
	return true
}

func parseProblems2(lines []string) [][]int {
	maxLen := 0
	for _, l := range lines { if len(l) > maxLen { maxLen = len(l) } }
	var problems [][]int
	var current []int
	for col := 0; col < maxLen; col++ {
		if isAllSpaces2(lines, col) {
			if len(current) > 0 { problems = append(problems, current); current = nil }
		} else { current = append(current, col) }
	}
	if len(current) > 0 { problems = append(problems, current) }
	return problems
}

func solve1(lines []string) int64 {
	lines = padLines2(lines)
	problems := parseProblems2(lines)
	numRows := len(lines) - 1
	opRow := lines[len(lines)-1]
	var total int64
	for _, cols := range problems {
		opChar := ' '
		for _, c := range cols { if c < len(opRow) && opRow[c] != ' ' { opChar = rune(opRow[c]); break } }
		var nums []int64
		for row := 0; row < numRows; row++ {
			line := lines[row]
			var sb strings.Builder
			for _, c := range cols { if c < len(line) { sb.WriteByte(line[c]) } else { sb.WriteByte(' ') } }
			str := strings.TrimSpace(sb.String())
			if str == "" { continue }
			n, _ := strconv.ParseInt(str, 10, 64)
			nums = append(nums, n)
		}
		var r int64
		if opChar == '+' { for _, n := range nums { r += n } } else { r = 1; for _, n := range nums { r *= n } }
		total += r
	}
	return total
}

func solve2(lines []string) int64 {
	lines = padLines2(lines)
	problems := parseProblems2(lines)
	numRows := len(lines) - 1
	opRow := lines[len(lines)-1]
	var total int64
	for _, cols := range problems {
		opChar := ' '
		for _, c := range cols { if c < len(opRow) && opRow[c] != ' ' { opChar = rune(opRow[c]); break } }
		var nums []int64
		for ci := len(cols)-1; ci >= 0; ci-- {
			col := cols[ci]
			var sb strings.Builder
			for row := 0; row < numRows; row++ {
				line := lines[row]
				if col < len(line) { sb.WriteByte(line[col]) } else { sb.WriteByte(' ') }
			}
			str := strings.TrimSpace(sb.String())
			if str == "" { continue }
			n, err := strconv.ParseInt(str, 10, 64)
			if err != nil {
				str2 := strings.ReplaceAll(str, " ", "")
				if str2 != "" { n, _ = strconv.ParseInt(str2, 10, 64) }
			}
			nums = append(nums, n)
		}
		var r int64
		if opChar == '+' { for _, n := range nums { r += n } } else { r = 1; for _, n := range nums { r *= n } }
		total += r
	}
	return total
}

func main() {
	lines := readLines2("example.txt")
	fmt.Println("Part 1:", solve1(lines), "(expected 4277556)")
	fmt.Println("Part 2:", solve2(lines), "(expected 3263827)")
}
