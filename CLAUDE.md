# Advent of Code 2025 — Claude Code Benchmark

## Purpose
Bachelor thesis project: benchmarking Claude Code's ability to solve Advent of Code 2025 challenges autonomously in Go. Every solution attempt, failure, and iteration is research data.

## Stack
- **Language**: Go 1.23+
- **Test**: `go test ./...`
- **Run solution**: `cd dayXX && go run .`
- **Lint**: `golangci-lint run ./...`

## Project Structure
```
aoc-2025/
├── CLAUDE.md              # This file
├── go.mod                 # Module: github.com/<user>/aoc-2025
├── go.sum
├── utils/                 # Shared utilities (parsing, grid, math)
│   ├── parse.go           # Input parsing helpers
│   ├── grid.go            # 2D grid operations
│   ├── math.go            # GCD, LCM, etc.
│   └── utils_test.go
├── day01/
│   ├── main.go            # Solution (Part 1 + Part 2)
│   ├── main_test.go       # Tests using example input
│   ├── input.txt          # Puzzle input (gitignored)
│   ├── example.txt        # Example input from problem description
│   └── PROBLEM.md         # Problem statement (copy-pasted)
├── day02/
│   └── ...                # Same structure
├── docs/
│   ├── thesis-log.jsonl   # Machine-readable research log (appended by hook)
│   └── observations.md    # Human-readable notes & analysis
└── .claude/
    ├── settings.json
    ├── commands/
    └── skills/
```

## Workflow Rules
1. **One folder per day**: `dayXX/` with zero-padded number (day01, day02, …, day25).
2. **Always create these files** for every new day:
   - `main.go` — package main, reads `input.txt`, prints Part 1 and Part 2 answers
   - `main_test.go` — tests both parts using `example.txt`
   - `example.txt` — the small example from the problem description
   - `PROBLEM.md` — the full problem statement as given
3. **input.txt is never committed** — it is in `.gitignore`.
4. **Reuse `utils/`** — check existing helpers before writing new parsing or math code.
5. **Tests first** — write tests against the example input before solving with real input.
6. **No external dependencies** beyond the standard library and `utils/`.

## Code Style
- `gofmt` is law. No exceptions.
- Exported functions get a one-line doc comment.
- Error handling: `log.Fatal(err)` in main, return errors in library code.
- Prefer clarity over cleverness — this code is research data, readability matters.

## Research Logging
After completing each day (success or failure), append a JSON line to `docs/thesis-log.jsonl`:
```json
{
  "day": 5,
  "part1_correct": true,
  "part2_correct": false,
  "attempts": 3,
  "duration_minutes": 12,
  "failure_category": "off_by_one",
  "notes": "Mishandled boundary in BFS",
  "model": "opus-4",
  "timestamp": "2025-12-05T14:30:00Z"
}
```
Failure categories: `correct`, `wrong_answer`, `off_by_one`, `timeout`, `misread_problem`, `wrong_algorithm`, `edge_case`, `parsing_error`, `other`

## When Compacting
Always preserve: the current day being worked on, any failing test output, and the list of completed days with their pass/fail status.
