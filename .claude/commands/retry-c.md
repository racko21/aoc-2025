---
description: Retry a failed C AoC day with a fresh approach. Usage /retry-c <day_number>
allowed-tools: Read(c/**), Write(c/**), Bash(cc *), Bash(gcc *), Bash(clang *), Bash(make *), Bash(cd c/* && *), Bash(valgrind *), Bash(cppcheck *), Glob(c/**), Grep(c/**)
---

## CRITICAL — Solve from scratch, no porting

You must solve this problem in C as if you have never seen any other
solution to it. Do NOT read, search for, glob, grep, or open any file
outside the `c/` directory for this task — specifically, never read
`dayNN/main.go`, any other language's solution, or any prior porting
attempt. If you have any memory of how this exact AoC 2025 problem was
solved in Go, set that memory aside and re-derive the algorithm
independently in C. Reviewing the previous *C* attempt below (Step 1) is
fine and expected — that is not porting, it is iterating on your own prior
C work.

This boundary is enforced by a PreToolUse hook (`.claude/hooks/c-arm-isolation.sh`)
that blocks access to any `*.go` file outside `c/` while a session marker
is present. Run `mkdir -p .claude && touch .claude/.c-arm-session` as your
very first action.

# Retry C Advent of Code 2025 — Day $1

This day was attempted before in C and failed. Take a different approach.

## Step 1: Review Previous Attempt
- Read `c/day$1/main.c` to understand the previous approach
- Read `c/day$1/test_main.c` to see what tests exist
- Check `docs/thesis-log.jsonl` for `"day": $1, "language": "c"` entries —
  note the failure category and notes
- Read `c/day$1/PROBLEM.md` carefully — the previous failure may have been
  a misread

## Step 2: Diagnose
- Identify WHY the previous attempt failed
- Consider: wrong algorithm? edge case? buffer overflow? integer overflow?
  off-by-one in loop bounds? misread problem?
- Document your diagnosis as a comment block in the new `main.c` before
  writing any code. This diagnosis is research data.

## Step 3: Fresh Solution
- If the algorithm was wrong, choose a fundamentally different approach
- If it was an edge case or bug, add specific test cases for those edges
- Save the old code: `cp c/day$1/main.c c/day$1/main_v1.c` before overwriting
  (keep it for comparison — the diff between v1 and v2 is data)
- Implement the new solution in `c/day$1/main.c`

## Step 4: Test and Verify
- Build and run tests: `make -C c day=$1 test`
- Run with real input: `make -C c day=$1 release`
- Report both answers

## Step 5: Measure (same as /solve-c Steps 6–7)
- Runtime: `./c/day$1/main 1` and `./c/day$1/main 2` (release build)
- Memory: `/usr/bin/time -v ./c/day$1/main 1/2 2>&1 | grep "Maximum resident"`
- Leaks: `valgrind --leak-check=full ./c/day$1/main 2>&1 | tail -10`
- Quality: compiler warnings, cppcheck, LOC

## Step 6: Log the Retry
- Append a NEW entry to `docs/thesis-log.jsonl` (do NOT overwrite the old
  one — both attempts are data for RQ3)
- Add `"retry": true` and `"previous_failure": "<category from first attempt>"`
  fields to the new entry
- `"attempts"` field: total attempts so far across all entries for this day
- Note in `"notes"` what changed in the approach
- Also append to `docs/observations.md` under `## C Implementation`, noting
  the failure-and-retry arc

## Step 7: End isolation
Run `rm -f .claude/.c-arm-session`.

## Constraints
- Same as /solve-c: C17, stdlib + c/utils/ only, no third-party libraries
- Keep `main_v1.c` — do not delete it
- Be honest about the attempt count and failure category
