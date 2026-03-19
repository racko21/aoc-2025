---
description: Log results for a completed AoC day. Usage /log <day_number>
allowed-tools: Read, Write, Bash
---

# Log Results for Day $1

Review the current state of `day$1/`:

1. Check if tests pass: `cd day$1 && go test -v`
2. Check if solution runs: `cd day$1 && go run .`
3. Read `day$1/PROBLEM.md` to understand what was asked

Then append a JSON line to `docs/thesis-log.jsonl` with:
- `day`: $1
- `part1_correct`: true/false based on test results
- `part2_correct`: true/false based on test results
- `attempts`: ask the user how many attempts were needed, or estimate from conversation
- `duration_minutes`: ask the user or estimate
- `failure_category`: one of: correct, wrong_answer, off_by_one, timeout, misread_problem, wrong_algorithm, edge_case, parsing_error, other
- `notes`: a brief description of what happened, especially any struggles or interesting observations
- `model`: the model that was used (ask if unsure)
- `timestamp`: current UTC time

Also update `docs/observations.md` with a brief human-readable summary of this day.
