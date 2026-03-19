---
description: Retry a failed AoC day with a fresh approach. Usage /retry <day_number>
allowed-tools: Read, Write, Bash, Grep, Glob
---

# Retry Advent of Code 2025 — Day $1

This day was attempted before and failed. Take a different approach.

## Step 1: Review Previous Attempt
- Read `day$1/main.go` to understand the previous approach
- Read `day$1/main_test.go` to see what tests exist
- Check `docs/thesis-log.jsonl` for the failure category and notes
- Read `day$1/PROBLEM.md` carefully — the previous failure may have been a misread

## Step 2: Diagnose
- Identify WHY the previous attempt failed
- Consider: Was the algorithm wrong? Was there an edge case? Was the problem misunderstood?
- Document your diagnosis before writing any code

## Step 3: Fresh Solution
- If the algorithm was wrong, choose a fundamentally different approach
- If it was an edge case, add specific test cases for those edges
- Keep the old code commented out or in a `main_v1.go` for thesis comparison

## Step 4: Test and Verify
- Run all tests: `cd day$1 && go test -v`
- Run with real input
- Report answers

## Step 5: Log the Retry
- Append a NEW entry to `docs/thesis-log.jsonl` (don't overwrite the old one — both attempts are data)
- Add `"retry": true` and `"previous_failure": "<category>"` fields
- Note what changed in the approach
