package utils

import (
	"bufio"
	"log"
	"os"
	"strconv"
	"strings"
)

// ReadLines reads a file and returns all lines as a string slice.
func ReadLines(path string) []string {
	file, err := os.Open(path)
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	var lines []string
	scanner := bufio.NewScanner(file)
	scanner.Buffer(make([]byte, 1024*1024), 1024*1024)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}
	return lines
}

// ReadString reads entire file as a single string, trimmed.
func ReadString(path string) string {
	data, err := os.ReadFile(path)
	if err != nil {
		log.Fatal(err)
	}
	return strings.TrimSpace(string(data))
}

// Atoi converts string to int, panics on error.
func Atoi(s string) int {
	n, err := strconv.Atoi(strings.TrimSpace(s))
	if err != nil {
		log.Fatalf("Atoi(%q): %v", s, err)
	}
	return n
}

// SplitInts splits a string by a separator and returns ints.
func SplitInts(s, sep string) []int {
	parts := strings.Split(s, sep)
	nums := make([]int, 0, len(parts))
	for _, p := range parts {
		p = strings.TrimSpace(p)
		if p == "" {
			continue
		}
		nums = append(nums, Atoi(p))
	}
	return nums
}

// Abs returns absolute value of an int.
func Abs(x int) int {
	if x < 0 {
		return -x
	}
	return x
}

// GCD returns greatest common divisor.
func GCD(a, b int) int {
	for b != 0 {
		a, b = b, a%b
	}
	return a
}

// LCM returns least common multiple.
func LCM(a, b int) int {
	return a / GCD(a, b) * b
}

// Min returns the smaller of two ints.
func Min(a, b int) int {
	if a < b {
		return a
	}
	return b
}

// Max returns the larger of two ints.
func Max(a, b int) int {
	if a > b {
		return a
	}
	return b
}
