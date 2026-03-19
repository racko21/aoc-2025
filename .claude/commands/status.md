---
description: Show progress overview across all AoC days
allowed-tools: Read, Bash, Grep, Glob
---

# Advent of Code 2025 — Progress Report

1. Read `docs/thesis-log.jsonl` and parse all entries
2. For each completed day, show: day number, part1 pass/fail, part2 pass/fail, attempts, failure category
3. Calculate summary stats:
   - Total days attempted
   - Part 1 success rate
   - Part 2 success rate
   - Most common failure categories
   - Average attempts per day
4. List which days (1-25) have NOT been attempted yet
5. Show any patterns (e.g. "failures increase after day 15", "parsing errors cluster on grid problems")

Format as a clean table. This helps the user write their thesis analysis.
