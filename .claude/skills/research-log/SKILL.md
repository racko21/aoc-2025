---
name: research-log
description: >
  Log research observations for the bachelor thesis. Use whenever a puzzle is completed,
  a failure is analyzed, a retry is attempted, or any notable behavior from Claude Code
  is observed. Also use when the user mentions thesis, logging, documenting, or observations.
---

# Research Logging for Bachelor Thesis

## When to Log
- After every puzzle attempt (success or failure)
- When Claude Code exhibits interesting behavior (hallucination, misread, creative solution)
- When a retry uses a fundamentally different approach
- When a pattern emerges across multiple days

## JSONL Entry Format (append to docs/thesis-log.jsonl)
```json
{
  "day": <int>,
  "part1_correct": <bool>,
  "part2_correct": <bool>,
  "attempts": <int>,
  "duration_minutes": <int>,
  "failure_category": "<string>",
  "notes": "<string — be specific, this is research data>",
  "model": "<string — e.g. opus-4, sonnet-4>",
  "algorithm_used": "<string — e.g. BFS, DP, simulation, brute_force>",
  "problem_type": "<string — e.g. grid, parsing, math, graph, simulation, string>",
  "lines_of_code": <int>,
  "used_utils": <bool>,
  "timestamp": "<ISO 8601 UTC>",
  "retry": <bool, optional>,
  "previous_failure": "<string, optional>"
}
```

## Failure Categories (use exactly these strings)
- `correct` — solved correctly
- `wrong_answer` — logic error producing incorrect result
- `off_by_one` — boundary/counting error
- `timeout` — solution too slow for real input
- `misread_problem` — misunderstood what was being asked
- `wrong_algorithm` — correct understanding but chose an approach that doesn't work
- `edge_case` — works on example but fails on edge case in real input
- `parsing_error` — input parsing bug
- `other` — explain in notes

## Qualitative Notes (append to docs/observations.md)
Under the current date heading, add a brief paragraph covering:
- What made this problem easy or hard for AI?
- Did Claude Code need hints or corrections?
- Any surprising behavior worth discussing in the thesis?
