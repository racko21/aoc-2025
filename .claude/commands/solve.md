---
description: Solve an Advent of Code day. Usage /solve <day_number>
allowed-tools: Read, Write, Bash, Grep, Glob
---

# Solve Advent of Code 2025 — Day $1

## Step 1: Setup
- Create directory `day$1/` if it doesn't exist
- Read the problem from `day$1/PROBLEM.md` (it should already be there — if not, ask the user to paste the problem statement and save it there)
- Extract the example input from the problem and save to `day$1/example.txt`
- Confirm `day$1/input.txt` exists (the real puzzle input)

## Step 2: Understand
- Summarize the problem in 2-3 sentences in a code comment at the top of main.go
- Identify the algorithm/data structure needed
- Check `utils/` for reusable helpers (parsing, grid, math)

## Step 3: Test First
- Create `day$1/main_test.go` with tests for Part 1 using the example input and expected answer from the problem description
- Run `cd day$1 && go test -v -run TestPart1` — make sure the test structure compiles

## Step 4: Solve Part 1
- Implement the solution in `day$1/main.go`
- Run tests until they pass
- Run with real input: `cd day$1 && go run . < input.txt`
- Report the answer

## Step 5: Solve Part 2
- Add Part 2 test to `main_test.go`
- Extend or refactor the solution
- Run tests, then run with real input
- Report the answer

## Step 6: Log Results
- Append results to `docs/thesis-log.jsonl` following the format in CLAUDE.md
- If any part failed, categorize the failure honestly
- Run `gofmt -w day$1/` and `cd day$1 && go vet ./...`

## Constraints
- Use ONLY Go standard library + project `utils/` package
- Always read input from file, never hardcode
- Print answers as: `fmt.Println("Part 1:", answer1)` and `fmt.Println("Part 2:", answer2)`
