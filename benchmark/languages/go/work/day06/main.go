package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func readLines(filename string) []string {
	f, err := os.Open(filename)
	if err != nil {
		panic(err)
	}
	defer f.Close()

	var lines []string
	scanner := bufio.NewScanner(f)
	scanner.Buffer(make([]byte, 1024*1024), 1024*1024)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	// Remove trailing empty lines
	for len(lines) > 0 && strings.TrimSpace(lines[len(lines)-1]) == "" {
		lines = lines[:len(lines)-1]
	}
	return lines
}

// padLines pads all lines to the same length
func padLines(lines []string) []string {
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

// isAllSpaces checks if all lines have a space at column col
func isAllSpaces(lines []string, col int) bool {
	for _, l := range lines {
		if col < len(l) && l[col] != ' ' {
			return false
		}
	}
	return true
}

// parseProblems splits the grid into problems.
// Each problem is a slice of column indices (contiguous non-all-space columns).
func parseProblems(lines []string) [][]int {
	if len(lines) == 0 {
		return nil
	}
	maxLen := 0
	for _, l := range lines {
		if len(l) > maxLen {
			maxLen = len(l)
		}
	}

	var problems [][]int
	var current []int
	for col := 0; col < maxLen; col++ {
		if isAllSpaces(lines, col) {
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

// Part 1: Each problem's columns together form numbers (read normally as horizontal text).
// The last row is the operator. Numbers are in rows 0..numRows-2.
// Extract the text for each number row by taking the columns of that problem,
// then parse the trimmed string as a number.
func solvePart1(lines []string) int64 {
	lines = padLines(lines)
	problems := parseProblems(lines)

	numRows := len(lines) - 1 // last row is operators
	opRow := lines[len(lines)-1]

	var grandTotal int64
	for _, cols := range problems {
		// Get operator
		opChar := ' '
		for _, c := range cols {
			if c < len(opRow) && opRow[c] != ' ' {
				opChar = rune(opRow[c])
				break
			}
		}

		// Get numbers from rows 0..numRows-1
		var numbers []int64
		for row := 0; row < numRows; row++ {
			line := lines[row]
			// Extract the substring for these columns
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
			n, err := strconv.ParseInt(str, 10, 64)
			if err != nil {
				continue
			}
			numbers = append(numbers, n)
		}

		if len(numbers) == 0 {
			continue
		}

		var result int64
		if opChar == '+' {
			for _, n := range numbers {
				result += n
			}
		} else if opChar == '*' {
			result = 1
			for _, n := range numbers {
				result *= n
			}
		}
		grandTotal += result
	}
	return grandTotal
}

// Part 2: Each problem is read right-to-left column by column.
// Each column (within the problem, in the number rows) gives one number,
// where the most significant digit is at the top and least significant at the bottom.
// So each character in a column is one digit of that number.
// Columns of a problem are read right-to-left.
func solvePart2(lines []string) int64 {
	lines = padLines(lines)
	problems := parseProblems(lines)

	numRows := len(lines) - 1 // last row is operators
	opRow := lines[len(lines)-1]

	var grandTotal int64
	for _, cols := range problems {
		// Get operator
		opChar := ' '
		for _, c := range cols {
			if c < len(opRow) && opRow[c] != ' ' {
				opChar = rune(opRow[c])
				break
			}
		}

		// Read columns right-to-left; each column is one number
		var numbers []int64
		for ci := len(cols) - 1; ci >= 0; ci-- {
			col := cols[ci]
			// Collect digit characters from this column, top to bottom (number rows only)
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
			// Each character in this column is a digit; they form the number top-to-bottom
			// TrimSpace removes leading/trailing spaces (from shorter numbers)
			n, err := strconv.ParseInt(str, 10, 64)
			if err != nil {
				// Remove all spaces (in case of internal spaces for some reason)
				str2 := strings.ReplaceAll(str, " ", "")
				if str2 == "" {
					continue
				}
				n2, err2 := strconv.ParseInt(str2, 10, 64)
				if err2 != nil {
					continue
				}
				n = n2
			}
			numbers = append(numbers, n)
		}

		if len(numbers) == 0 {
			continue
		}

		var result int64
		if opChar == '+' {
			for _, n := range numbers {
				result += n
			}
		} else if opChar == '*' {
			result = 1
			for _, n := range numbers {
				result *= n
			}
		}
		grandTotal += result
	}
	return grandTotal
}

func main() {
	lines := readLines("input.txt")

	part := ""
	if len(os.Args) > 1 {
		part = os.Args[1]
	}

	if part == "1" || part == "" {
		fmt.Println("Part 1:", solvePart1(lines))
	}
	if part == "2" || part == "" {
		fmt.Println("Part 2:", solvePart2(lines))
	}
}
