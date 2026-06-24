---
description: Solve an Advent of Code day in C. Usage /solve-c <day_number>
allowed-tools: Read(c/**), Write(c/**), Bash(cc *), Bash(gcc *), Bash(clang *), Bash(make *), Bash(cd c/* && *), Bash(valgrind *), Bash(cppcheck *), Glob(c/**), Grep(c/**)
---

## CRITICAL — Solve from scratch, no porting

You must solve this problem in C as if you have never seen any other
solution to it. Do NOT read, search for, glob, grep, or open any file
outside the `c/` directory for this task — specifically, never read
`dayNN/main.go`, any other language's solution, or any prior porting
attempt. If you have any memory (from earlier in this conversation, from a
prior turn, or from training data) of how this exact AoC 2025 problem was
solved in Go, set that memory aside and re-derive the algorithm and parsing
approach independently in C. Read ONLY `c/dayNN/PROBLEM.md` and
`c/dayNN/example.txt` to understand the problem — these are the only
legitimate inputs.

If you find yourself about to write code that closely mirrors a Go solution
you're aware of (same variable names, same exact algorithm choice, same
structuring), stop and ask yourself: would I have arrived at this
independently? If unsure, proceed with your own first-principles approach
to the C version rather than the recalled one.

This boundary is enforced by a PreToolUse hook (`.claude/hooks/c-arm-isolation.sh`)
that blocks access to any `*.go` file outside `c/` while a session marker
is present — created in Step 0 below and removed in Step 9. Do not attempt
to work around the marker file or the hook.

# Solve Advent of Code 2025 in C — Day $1

## Step 0: Record start time and start isolation
Run `mkdir -p .claude && touch .claude/.c-arm-session` as your very first
action, before reading or writing anything else, to activate the
Go-isolation hook for the rest of this session.

Note the wall-clock time right now. Compute `duration_minutes` at Step 9
as (end − start). Log `duration_estimated: false` since this is live.

## Step 1: Setup
- `mkdir -p c/day$1` if needed
- Copy files if absent: `cp day$1/PROBLEM.md c/day$1/ && cp day$1/example.txt c/day$1/`
- If `day$1/input.txt` exists: `cp day$1/input.txt c/day$1/`
  Otherwise STOP and ask the user to supply it — do not fabricate input.
- Read `c/day$1/PROBLEM.md`

## Step 2: Understand
- Summarize the problem in 2-3 sentences as a block comment at the top of `main.c`
- Identify the algorithm/data structure needed
- Read `c/utils/utils.h` and check for applicable helpers before writing new ones

## Step 3: Test First
Create `c/day$1/test_main.c` with Part 1 assertions against `example.txt`:
```c
#include <assert.h>
#include <stdio.h>
#include "utils/utils.h"

long long solve_part1(const char *path);
long long solve_part2(const char *path);

int main(void) {
    long long p1 = solve_part1("c/day$1/example.txt");
    assert(p1 == EXPECTED_PART1);
    printf("Part 1 OK: %lld\n", p1);
    return 0;
}
```
Build: `make -C c day=$1 test`
(Expect link failure until Part 1 is in main.c — fix compile errors first.)

## Step 4: Solve Part 1
Implement `c/day$1/main.c`. The binary MUST support a part-selector arg and
instrument timing with `clock_gettime(CLOCK_MONOTONIC, ...)`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils/utils.h"

/* <2-3 sentence problem summary> */

static long long solve_part1(const char *path) { /* ... */ }
static long long solve_part2(const char *path) { /* ... */ }

static double elapsed_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000.0 + (b.tv_nsec - a.tv_nsec) / 1e6;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc > 1) part = atoi(argv[1]);

    struct timespec t0, t1;
    if (part == 0 || part == 1) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans1 = solve_part1("c/day$1/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("c/day$1/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }
    return 0;
}
```

- Build+run: `make -C c day=$1 run` (uses -Wall -Wextra -Wpedantic, -O0 -g)
- Resolve ALL warnings. If intentionally left, note why in code + log.
- Run tests: `make -C c day=$1 test`
- Run with real input: `make -C c day=$1 run`
- Report the answer

## Step 5: Solve Part 2
- Extend `test_main.c` with Part 2 assertion
- Extend `main.c`
- `make -C c day=$1 test` then `make -C c day=$1 run`
- Report the answer

## Step 6: Measure — runtime and memory (RQ1)
First rebuild in release mode: `make -C c day=$1 release`

**Runtime** (from stderr output above):
- `./c/day$1/main 1 2>&1 >/dev/null` — capture `runtime_ms_part1`
- `./c/day$1/main 2 2>&1 >/dev/null` — capture `runtime_ms_part2`

**Peak memory** (GNU time -v):
```
/usr/bin/time -v ./c/day$1/main 1 2>&1 | grep "Maximum resident set size"
/usr/bin/time -v ./c/day$1/main 2 2>&1 | grep "Maximum resident set size"
```
Report in KB as `peak_memory_kb_part1` and `peak_memory_kb_part2`.

**Leak detection**: Not available (no valgrind/ASAN). Set `memory_leaks_detected: null`.
Note any obvious leaks found by code inspection in `notes`.

## Step 7: Measure — code quality (RQ1)
- **Warnings**: `gcc -std=c17 -Wall -Wextra -Wpedantic -fsyntax-only c/day$1/main.c -Ic 2>&1 | grep -c "warning:"` → `compiler_warnings`
- **cppcheck**: Not available. Set `cppcheck_warnings: null`.
- **Complexity**: Not available (no lizard). Set `cyclomatic_complexity_avg: null`.
- **LOC**: `grep -cv "^[[:space:]]*/\|^[[:space:]]*$" c/day$1/main.c` → `lines_of_code`
- **Compiler**: `gcc --version | head -1` → record in `compiler` field

## Step 8: Measure — token cost (RQ2)
At session end check the Claude Code usage report and transcribe:
- `input_tokens`, `output_tokens`, `cache_read_input_tokens`, `cache_creation_input_tokens`
- `cost_usd`
- `cost_calculation_note: "Claude Code session usage report, one session per day"`

If unavailable: set all cost fields to `null` with note "session report unavailable".

## Step 9: Log results (RQ3)
Append ONE JSON line to `docs/thesis-log.jsonl`:
```json
{
  "day": $1, "language": "c",
  "part1_correct": true, "part2_correct": true,
  "attempts": 1, "duration_minutes": <live>,
  "duration_estimated": false,
  "failure_category": "correct",
  "notes": "...",
  "model": "claude-sonnet-4-6",
  "algorithm_used": "...", "problem_type": "...",
  "lines_of_code": <count>, "used_utils": false,
  "timestamp": "<UTC ISO-8601>",
  "input_tokens": null, "output_tokens": null,
  "cache_read_input_tokens": null, "cache_creation_input_tokens": null,
  "cost_usd": null,
  "cost_calculation_note": "fill from session report at end",
  "runtime_ms_part1": <value>, "runtime_ms_part2": <value>,
  "peak_memory_kb_part1": <value>, "peak_memory_kb_part2": <value>,
  "memory_leaks_detected": null,
  "compiler": "gcc (GCC) 15.2.1 20260123 (Red Hat 15.2.1-7)",
  "optimization_flag": "-O2",
  "compiler_warnings": 0,
  "cppcheck_warnings": null,
  "cyclomatic_complexity_avg": null
}
```
Be honest about `attempts` and `failure_category`. Do not collapse a
3-attempt day into "correct" — all attempts are data for RQ3.

Also append a dated paragraph to `docs/observations.md` under
`## C Implementation` describing algorithm, C-specific friction, and
differences from the Go version.

## Step 10: End isolation
Run `rm -f .claude/.c-arm-session` as your final action, once Step 9's log
entry has been written.

## Constraints
- C17 + stdlib + `c/utils/` only. No third-party libraries.
- Always read input from file. Never hardcode values.
- Print: `printf("Part 1: %lld\n", ans1);` and `printf("Part 2: %lld\n", ans2);`
- Always instrument timing with `clock_gettime(CLOCK_MONOTONIC, ...)`.
