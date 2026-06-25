# Benchmark harness — setup & usage

This harness solves AoC 2025 days autonomously via the Anthropic API, one
isolated session per (language, day), and records RQ1 (runtime/memory/quality),
RQ2 (exact token cost), and RQ3 (attempts/failure) data into
`results/thesis-log.jsonl`.

## What makes a run valid

- **Isolation**: each (language, day) session runs in
  `languages/<lang>/work/dayNN/`, which contains ONLY a copy of
  `shared/dayNN/PROBLEM.md` and `input.txt`. No other language's code exists in
  that sandbox, and each day starts with a fresh message history. Porting from
  another language is impossible because the other code is not present.
- **The model never sees the answer key**: `shared/dayNN/real_answers.txt` is
  read by the harness only. Submission feedback tells the model "Part N correct
  / incorrect" but never the value, so it cannot hardcode answers.
- **Cost is exact**: token counts come from the API `usage` field per call,
  summed per day — no manual transcription.

## One-time setup

1. Python deps for the harness itself (not the solutions):
   ```bash
   pip install anthropic pyyaml
   ```

2. Set your key:
   ```bash
   export ANTHROPIC_API_KEY=sk-ant-...
   ```

3. Confirm the shared assets exist for every day you'll run:
   - `shared/dayNN/PROBLEM.md`  (both parts' text where part 2 is unlocked)
   - `shared/dayNN/input.txt`   (your real puzzle input; gitignored)
   - `shared/dayNN/real_answers.txt`  (bare values, one per line: part1 then
     part2; a single line if the day has no part 2)

4. Verify pricing in `config.yaml` against the current Anthropic published
   rates. The committed values are placeholders; `cost_usd` is only as accurate
   as those numbers.

5. Make the language scripts executable (once):
   ```bash
   chmod +x languages/*/build.sh languages/*/test.sh languages/*/loc.sh
   ```

## Validate before spending budget

```bash
python runner.py --config config.yaml --dry-run
```

This checks every required file is present and the key is set, then exits
before any API call. Fix anything it reports.

## Pilot a single day first

Strongly recommended: run ONE day end to end, inspect it, then scale.

```bash
python runner.py --config config.yaml --lang python --days 1
```

Then inspect:
- `results/thesis-log.jsonl` — the new line for python/day1
- `runs/python_day01_*/transcript.json` — the full session (did it solve from
  scratch? did the loop behave?)
- `runs/python_day01_*/usage.json` — token/cost for that day
- `results/isolation-audit.log` — every file/bash action the model took

If that looks right, run the rest.

## Full run

```bash
python runner.py --config config.yaml          # all languages/days in config
# or scope it:
python runner.py --config config.yaml --lang python
python runner.py --config config.yaml --lang python --days 2,3,4
```

The run is resumable in practice by listing only the days you still need —
each day appends independently, so a crash mid-run loses only the in-flight
day. (Re-running a day appends a second line for it; de-dup at analysis time or
remove the stale line.)

## Adding a language later (e.g. re-running C for its cost data)

1. Ensure `languages/c/{LANG_SPEC.md,build.sh,test.sh,loc.sh}` are filled in
   (build.sh compiles with the fixed -O2 flag and emits COMPILER=/OPTFLAG=/
   WARNINGS=; test.sh runs one part and emits runtime_ms:/peak_kb:).
2. Add `c` to `languages:` in config.yaml (or use `--lang c`).
3. Run. The harness is otherwise unchanged.

## Output schema

Each line in `results/thesis-log.jsonl` has the same extended schema as the
existing C arm, plus exact cost fields. `language` distinguishes arms. Merge
with the existing Go/C log at analysis time (filter by `language`).

## Known cross-language measurement nuance

For compiled arms (C/Go) each part's runtime is measured around that part's
solve call. For interpreted Python, `solution.py` prints both parts in one
process, so per-part runtime is the whole-program time from a single run.
Document this in the thesis methodology — Python runtime includes interpreter
startup + both parts' work, and should be compared to C/Go with that caveat,
or normalized (e.g. compare only relative ordering within a language).
