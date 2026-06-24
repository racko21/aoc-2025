// Day 11: Reactor
// Count all paths from "you" to "out" in a directed acyclic device graph.
// Part 1: memoized DFS path count from "you" to "out".
package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strings"
	"time"
)

func parseGraph(filename string) map[string][]string {
	f, err := os.Open(filename)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()

	graph := make(map[string][]string)
	sc := bufio.NewScanner(f)
	for sc.Scan() {
		line := strings.TrimSpace(sc.Text())
		if line == "" {
			continue
		}
		parts := strings.SplitN(line, ": ", 2)
		node := parts[0]
		if len(parts) < 2 || parts[1] == "" {
			graph[node] = nil
			continue
		}
		graph[node] = strings.Fields(parts[1])
	}
	if err := sc.Err(); err != nil {
		log.Fatal(err)
	}
	return graph
}

func countPaths(graph map[string][]string, node string, memo map[string]int) int {
	if node == "out" {
		return 1
	}
	if v, ok := memo[node]; ok {
		return v
	}
	total := 0
	for _, next := range graph[node] {
		total += countPaths(graph, next, memo)
	}
	memo[node] = total
	return total
}

type state2 struct {
	node             string
	haveDac, haveFft bool
}

// countPathsRequired counts paths from node to "out" that visit both "dac" and "fft".
func countPathsRequired(graph map[string][]string, node string, haveDac, haveFft bool, memo map[state2]int) int {
	if node == "out" {
		if haveDac && haveFft {
			return 1
		}
		return 0
	}
	key := state2{node, haveDac, haveFft}
	if v, ok := memo[key]; ok {
		return v
	}
	total := 0
	for _, next := range graph[node] {
		total += countPathsRequired(graph, next, haveDac || next == "dac", haveFft || next == "fft", memo)
	}
	memo[key] = total
	return total
}

func solve(filename string) (int, int) {
	graph := parseGraph(filename)
	memo1 := make(map[string]int)
	part1 := countPaths(graph, "you", memo1)
	memo2 := make(map[state2]int)
	part2 := countPathsRequired(graph, "svr", false, false, memo2)
	return part1, part2
}

func main() {
	if len(os.Args) > 1 {
		switch os.Args[1] {
		case "1":
			start := time.Now()
			graph := parseGraph("input.txt")
			result := countPaths(graph, "you", make(map[string]int))
			fmt.Fprintf(os.Stderr, "runtime_ms: %.2f\n", float64(time.Since(start).Microseconds())/1000)
			fmt.Println("Part 1:", result)
			return
		case "2":
			start := time.Now()
			graph := parseGraph("input.txt")
			result := countPathsRequired(graph, "svr", false, false, make(map[state2]int))
			fmt.Fprintf(os.Stderr, "runtime_ms: %.2f\n", float64(time.Since(start).Microseconds())/1000)
			fmt.Println("Part 2:", result)
			return
		}
	}
	p1, p2 := solve("input.txt")
	fmt.Println("Part 1:", p1)
	fmt.Println("Part 2:", p2)
}
