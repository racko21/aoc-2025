---
name: aoc-patterns
description: >
  Common Advent of Code algorithm patterns in Go. Use whenever solving an AoC puzzle,
  implementing BFS/DFS, dynamic programming, grid traversal, parsing, interval math,
  or any competitive-programming-style problem. Also use when debugging a wrong answer.
---

# Advent of Code Algorithm Patterns (Go)

## Problem Classification — Read the problem, then decide:

| Signal in problem text | Likely approach |
|---|---|
| "shortest path", "fewest steps" | BFS (unweighted) or Dijkstra (weighted) |
| "count all paths", "how many ways" | DFS with memoization / DP |
| "simulate N steps" | Direct simulation; watch for cycles |
| "after 1000000000 steps" | Cycle detection (Floyd / seen-state map) |
| "ranges", "intervals" | Interval splitting, not brute force |
| "dependencies", "ordering" | Topological sort |
| "matching", "assignment" | Hungarian algorithm or bipartite matching |
| "maximize/minimize with constraints" | DP or greedy with proof |
| Grid with walls and movement | BFS on grid |
| "how many positions" on infinite-feeling space | BFS/flood fill with bounds |

## Common Gotchas (check these when you get a wrong answer)
1. **Off-by-one**: Fences vs fence posts. Count edges vs vertices.
2. **Integer overflow**: Use `int` (64-bit on modern Go), not `int32`.
3. **Input parsing**: Trailing newline, Windows `\r\n`, extra spaces.
4. **Map vs slice for sparse grids**: If coordinates go negative or are huge, use `map[Point]byte`.
5. **Visited set**: Must include direction/state, not just position, for problems where you can revisit with different state.
6. **Part 2 twist**: Part 2 often makes brute force infeasible. Look for mathematical shortcuts, cycle detection, or smart state representation.

## BFS Template (Go)
```go
type State struct {
    pos   utils.Point
    steps int
}

func bfs(grid [][]byte, start, end utils.Point) int {
    queue := []State{{start, 0}}
    visited := map[utils.Point]bool{start: true}
    for len(queue) > 0 {
        cur := queue[0]
        queue = queue[1:]
        if cur.pos == end {
            return cur.steps
        }
        for _, d := range utils.Dir4 {
            next := utils.Point{cur.pos.R + d.R, cur.pos.C + d.C}
            if utils.InBounds(next, len(grid), len(grid[0])) && !visited[next] && grid[next.R][next.C] != '#' {
                visited[next] = true
                queue = append(queue, State{next, cur.steps + 1})
            }
        }
    }
    return -1
}
```

## Cycle Detection Template
```go
func findCycle(simulate func(state State) State, initial State) (cycleStart, cycleLen int) {
    seen := map[State]int{}
    state := initial
    for step := 0; ; step++ {
        if prev, ok := seen[state]; ok {
            return prev, step - prev
        }
        seen[state] = step
        state = simulate(state)
    }
}
// Then: remaining = (targetSteps - cycleStart) % cycleLen
// Simulate cycleStart + remaining steps total
```

## Debugging Checklist
When a solution gives the wrong answer:
1. Re-read the problem statement — literally every word
2. Test with the example input first
3. Print intermediate state for small inputs
4. Check for off-by-one in bounds, loop limits, and result
5. Check if Part 2 changed any assumption from Part 1
6. Try edge cases: empty input, single element, max values
