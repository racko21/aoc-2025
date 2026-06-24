---
description: Show C arm progress overview across all AoC days
allowed-tools: Read(c/**), Read(docs/**), Bash(cd c/* && *), Glob(c/**), Grep(c/**), Grep(docs/**)
---

## CRITICAL — Solve from scratch, no porting

This command only reports on existing logs/measurements — it does not
write any C code. Even so, do NOT read, search for, glob, grep, or open
any `dayNN/main.go` or other Go solution file: this report compares the
Go and C arms using only `docs/thesis-log.jsonl`, which already has the
data needed for both arms. There is never a legitimate reason for this
command to open a `.go` file.

# Advent of Code 2025 — C Arm Progress Report

1. Read `docs/thesis-log.jsonl` and parse all entries where `"language": "c"`
   (entries without a `"language"` field are Go — skip them here)

2. For each completed C day, show:
   - Day number
   - Part 1 pass/fail
   - Part 2 pass/fail
   - Attempts
   - Failure category
   - Duration (minutes)
   - Runtime (ms, both parts)
   - Memory (KB, both parts)
   - Compiler warnings
   - Cost (USD)

3. Calculate C arm summary stats:
   - Total days attempted / 12
   - Part 1 success rate
   - Part 2 success rate
   - Most common failure categories
   - Average attempts per day
   - Average duration per day
   - Total cost so far (sum of cost_usd, skip nulls)
   - Total token usage

4. Compare against Go arm (entries without `"language"` field):
   - Average attempts: Go vs C
   - Average duration: Go vs C
   - Success rate: Go vs C
   - Rough runtime comparison where both arms completed the same day

5. List which C days (01–12) have NOT been attempted yet (no log entry with
   `"language": "c"`)

6. Flag any data quality issues:
   - Log entries with null cost fields (need filling from session report)
   - Days where `duration_estimated: true` (remind user to track live)
   - Missing measurement fields

Format as a clean table. This is thesis analysis data — be precise.
