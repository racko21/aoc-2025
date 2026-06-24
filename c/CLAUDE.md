# Advent of Code 2025 — C Benchmark Arm

## Purpose
Bachelor thesis project, C-language arm. This replicates the existing Go
benchmark (see ../CLAUDE.md and ../docs/) using identical problem inputs,
solved independently in C. Every attempt, failure, timing, token-cost, and
quality measurement is research data for three research questions:

- RQ1: language effect on runtime/memory efficiency and code quality
- RQ2: language effect on token cost to reach a correct solution
- RQ3: failure/iteration patterns vs. problem type vs. language

Do not optimize for "looking good" — log real attempts, real bugs, real
durations honestly. A correct-first-try day and a four-attempt day are
equally valuable data points.

## Verified Toolchain (checked 2026-06-23)
| Tool           | Status       | Version / Note                            |
|----------------|-------------|-------------------------------------------|
| gcc            | ✓ available  | GCC 15.2.1 (Red Hat 15.2.1-7)            |
| make           | ✓ available  | GNU Make 4.4.1                            |
| /usr/bin/time  | ✓ available  | GNU Time 1.9 (use -v for memory)          |
| clang-format   | ✗ absent     | Not installed — format manually           |
| clang          | ✗ absent     | gcc only                                  |
| valgrind       | ✗ absent     | Set memory_leaks_detected: null           |
| cppcheck       | ✗ absent     | Set cppcheck_warnings: null               |
| lizard         | ✗ absent     | Set cyclomatic_complexity_avg: null       |
| AddressSanitizer| ✗ absent    | libasan.so.8.0.0 missing                  |

## Stack
- **Language**: C (C17), compiled with `gcc` (GCC 15.2.1).
- **Build**: `make -C c day=NN <target>` from repo root. See Makefile.
- **Test**: minimal custom harness — `test_main.c` compiles standalone,
  asserts expected answers, exits non-zero on failure.
- **Lint**: `gcc -Wall -Wextra -Wpedantic -fsyntax-only` — count warnings.
  cppcheck is unavailable; log `cppcheck_warnings: null` for all days.
- **Memory**: `/usr/bin/time -v` for peak RSS. Leak detection unavailable
  (no valgrind, no ASAN). Log `memory_leaks_detected: null` for all days.
- **Formatting**: No clang-format available. Apply consistent style manually
  (see Code Style below). No PostToolUse hook for .c files.

## Project Structure
```
c/
├── CLAUDE.md
├── Makefile
├── utils/
│   ├── utils.h          # shared parsing/grid/math helpers
│   └── utils.c
├── day01/
│   ├── main.c            # Solution (Part 1 + Part 2), reads input.txt
│   ├── test_main.c        # Tests using example.txt
│   ├── input.txt           # Puzzle input (gitignored)
│   ├── example.txt         # Example input (copied from ../day01/)
│   └── PROBLEM.md          # Problem statement (copied from ../day01/, verbatim)
├── day02/ ... day12/
```

## Workflow Rules
1. **One folder per day**: `c/dayNN/` zero-padded, matching the Go structure.
2. **Always create these files** for every new day:
   - `main.c` — reads `input.txt`, prints `Part 1: <answer>` and
     `Part 2: <answer>` to stdout. Supports `./main 1` / `./main 2` for
     per-part timing.
   - `test_main.c` — asserts both parts against `example.txt`.
   - `example.txt`, `input.txt`, `PROBLEM.md` — already copied verbatim
     from the Go arm's day folder as part of the C arm setup. Never retype.
3. **input.txt is never committed** — `**/input.txt` in `.gitignore` covers
   this. Days 02 and 03 are missing input.txt in the repo (not present in
   Go arm either) — ask the user to supply them before solving those days.
4. **Reuse `c/utils/`** — check `utils/utils.h` before writing new parsing
   or math code. Grow it organically as days require new helpers.
5. **Tests first** — write and compile `test_main.c` before solving.
6. **No external dependencies** — C standard library + `c/utils/` only.
7. **Memory discipline**: every `malloc`/`calloc` must have a matching
   `free`. Leaks cannot be detected with available tools; document any
   obvious leaks in `notes` via code inspection.

## Code Style
- **Format**: No clang-format available. Use 4-space indentation, K&R brace
  style (opening brace on same line). Keep lines under 100 chars.
  Be consistent within each file — readability matters as much as the Go arm.
- Every function gets a one-line doc comment above its declaration.
- Error handling: check `malloc`/`fopen` returns; on failure,
  `fprintf(stderr, "error: ...\n"); exit(1);`
- Prefer clarity over cleverness — this is research data.

## Measurement Methods (fixed for all 12 days — do not change mid-run)

### Runtime
Instrument `clock_gettime(CLOCK_MONOTONIC, ...)` inside `main.c` around each
part's solve function. The binary accepts an optional arg: `./main 1` runs
and times Part 1 only; `./main 2` runs and times Part 2 only; `./main`
(no arg) runs both. Print timing to stderr: `fprintf(stderr, "runtime_ms: %.2f\n", ms)`.

Report as `runtime_ms_part1` and `runtime_ms_part2` in the log.

### Memory (peak RSS via /usr/bin/time)
```
/usr/bin/time -v ./c/dayNN/main 1 2>&1 | grep "Maximum resident set size"
/usr/bin/time -v ./c/dayNN/main 2 2>&1 | grep "Maximum resident set size"
```
Value is in KB. Report as `peak_memory_kb_part1` and `peak_memory_kb_part2`.

### Leak Detection
Not available (no valgrind, no ASAN). Set `memory_leaks_detected: null`
for all days. Note in `notes` if code inspection reveals obvious leaks.

### Build flag for all measurements
`-O2` via `make -C c day=NN release`. All 12 days must use this flag.
Never report timing from a `-O0` build in the log.

### Code Quality
- `compiler_warnings`: from `gcc -std=c17 -Wall -Wextra -Wpedantic -fsyntax-only c/dayNN/main.c 2>&1 | grep -c "warning:"`
- `cppcheck_warnings`: `null` for all days (tool unavailable)
- `cyclomatic_complexity_avg`: `null` for all days (lizard unavailable)
- `lines_of_code`: `grep -cv "^[[:space:]]*/\|^[[:space:]]*$" c/dayNN/main.c`

## Cost Tracking Method
**Method: fresh Claude Code session per day (approach 2 from setup instructions).**

Start a NEW Claude Code conversation for each day's `/solve-c NN` session.
At session end, the Claude Code session summary reports cumulative token
usage and USD cost. Transcribe `input_tokens`, `output_tokens`,
`cache_read_input_tokens`, `cache_creation_input_tokens`, and `cost_usd`
into the day's log line.

This isolates each day's cost and prevents prior-day context from being a
confound in RQ2 and RQ3.

If the per-session report is unavailable or ambiguous, set cost fields to
`null` with `cost_calculation_note: "session report unavailable"`.

## Research Logging
After completing each day, append ONE JSON line to `../docs/thesis-log.jsonl`
using the extended C schema (see root setup instructions). Required fields:
`language: "c"`, all timing/memory/quality/cost fields populated or
explicitly `null` with a note. Never omit a key entirely.

Also append a paragraph to `../docs/observations.md` under `## C Implementation`
(create the heading if absent), dated, describing algorithm, C-specific
friction, and comparison to the Go version.

## When Compacting
Preserve: current day being worked on, any failing compiler/test output,
list of completed C days with pass/fail status, running token totals.
