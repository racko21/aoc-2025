package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

func parseGraph(filename string) map[string][]string {
	f, err := os.Open(filename)
	if err != nil {
		panic(err)
	}
	defer f.Close()

	graph := make(map[string][]string)
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		// Format: "name: dest1 dest2 ..."
		colonIdx := strings.Index(line, ":")
		if colonIdx < 0 {
			continue
		}
		src := strings.TrimSpace(line[:colonIdx])
		rest := strings.TrimSpace(line[colonIdx+1:])
		if rest == "" {
			graph[src] = []string{}
			continue
		}
		dests := strings.Fields(rest)
		graph[src] = append(graph[src], dests...)
	}
	return graph
}

// Part 1: count paths from "you" to "out"
// Use memoization: memo[node] = number of paths from node to "out"
func countPaths(graph map[string][]string) int64 {
	memo := make(map[string]int64)
	var dfs func(node string) int64
	dfs = func(node string) int64 {
		if node == "out" {
			return 1
		}
		if v, ok := memo[node]; ok {
			return v
		}
		var total int64
		for _, next := range graph[node] {
			total += dfs(next)
		}
		memo[node] = total
		return total
	}
	return dfs("you")
}

// Part 2: count paths from "svr" to "out" that visit both "dac" and "fft"
// State: (node, visitedDac, visitedFft)
// Memoize on state
type state2 struct {
	node       string
	visitedDac bool
	visitedFft bool
}

func countPathsPart2(graph map[string][]string) int64 {
	memo := make(map[state2]int64)
	var dfs func(node string, vDac, vFft bool) int64
	dfs = func(node string, vDac, vFft bool) int64 {
		if node == "out" {
			if vDac && vFft {
				return 1
			}
			return 0
		}
		s := state2{node, vDac, vFft}
		if v, ok := memo[s]; ok {
			return v
		}
		var total int64
		for _, next := range graph[node] {
			nDac := vDac || (next == "dac")
			nFft := vFft || (next == "fft")
			total += dfs(next, nDac, nFft)
		}
		memo[s] = total
		return total
	}
	// Check if svr itself is dac or fft (unlikely but handle it)
	startDac := false
	startFft := false
	return dfs("svr", startDac, startFft)
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

	graph := parseGraph("input.txt")

	if part == 0 || part == 1 {
		fmt.Printf("Part 1: %d\n", countPaths(graph))
	}
	if part == 0 || part == 2 {
		fmt.Printf("Part 2: %d\n", countPathsPart2(graph))
	}
}
