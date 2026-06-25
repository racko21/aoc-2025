package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func readLinesFrom(filename string) []string {
	f, err := os.Open(filename)
	if err != nil {
		panic(err)
	}
	defer f.Close()

	var lines []string
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	return lines
}

func padLinesEx(lines []string) []string {
	maxLen := 0
	for _, l := range lines {
		if len(l) > maxLen {
			maxLen = len(l)
		}
	}
	result := make([]string, len(lines))
	for i, l := range lines {
		if len(l) < maxLen {
			result[i] = l + strings.Repeat(" ", maxLen-len(l))
		} else {
			result[i] = l
		}
	}
	return result
}

func isAllSpacesEx(lines []string, col int) bool {
	for _, l := range lines {
		if col < len(l) && l[col] != ' ' {
			return false
		}
	}
	return true
}

func parseProblemsEx(lines []string) [][]int {
	maxLen := 0
	for _, l := range lines {
		if len(l) > maxLen {
			maxLen = len(l)
		}
	}
	var problems [][]int
	var current []int
	for col := 0; col < maxLen; col++ {
		if isAllSpacesEx(lines, col) {
			if len(current) > 0 {
				problems = append(problems, current)
				current = nil
			}
		} else {
			current = append(current, col)
		}
	}
	if len(current) > 0 {
		problems = append(problems, current)
	}
	return problems
}

func main() {
	lines := readLinesFrom("example.txt")
	lines = padLinesEx(lines)
	problems := parseProblemsEx(lines)
	fmt.Printf("Found %d problems\n", len(problems))
	for i, p := range problems {
		fmt.Printf("Problem %d: cols %v\n", i+1, p)
	}

	numRows := len(lines) - 1
	opRow := lines[len(lines)-1]
	fmt.Println("opRow:", opRow)

	// Part 1
	var total1 int64
	for i, cols := range problems {
		opChar := ' '
		for _, c := range cols {
			if c < len(opRow) && opRow[c] != ' ' {
				opChar = rune(opRow[c])
				break
			}
		}
		var numbers []int64
		for row := 0; row < numRows; row++ {
			line := lines[row]
			var sb strings.Builder
			for _, c := range cols {
				if c < len(line) {
					sb.WriteByte(line[c])
				} else {
					sb.WriteByte(' ')
				}
			}
			str := strings.TrimSpace(sb.String())
			if str == "" {
				continue
			}
			n, _ := strconv.ParseInt(str, 10, 64)
			numbers = append(numbers, n)
		}
		var result int64
		if opChar == '+' {
			for _, n := range numbers { result += n }
		} else {
			result = 1
			for _, n := range numbers { result *= n }
		}
		fmt.Printf("P1 Problem %d: op=%c nums=%v result=%d\n", i+1, opChar, numbers, result)
		total1 += result
	}
	fmt.Println("Part 1 total:", total1, "(expected 4277556)")

	// Part 2
	var total2 int64
	for i, cols := range problems {
		opChar := ' '
		for _, c := range cols {
			if c < len(opRow) && opRow[c] != ' ' {
				opChar = rune(opRow[c])
				break
			}
		}
		var numbers []int64
		for ci := len(cols) - 1; ci >= 0; ci-- {
			col := cols[ci]
			var sb strings.Builder
			for row := 0; row < numRows; row++ {
				line := lines[row]
				if col < len(line) {
					sb.WriteByte(line[col])
				} else {
					sb.WriteByte(' ')
				}
			}
			str := strings.TrimSpace(sb.String())
			if str == "" {
				continue
			}
			n, err := strconv.ParseInt(str, 10, 64)
			if err != nil {
				str2 := strings.ReplaceAll(str, " ", "")
				if str2 != "" {
					n, _ = strconv.ParseInt(str2, 10, 64)
				}
			}
			numbers = append(numbers, n)
		}
		var result int64
		if opChar == '+' {
			for _, n := range numbers { result += n }
		} else {
			result = 1
			for _, n := range numbers { result *= n }
		}
		fmt.Printf("P2 Problem %d: op=%c nums=%v result=%d\n", i+1, opChar, numbers, result)
		total2 += result
	}
	fmt.Println("Part 2 total:", total2, "(expected 3263827)")
}
