#!/usr/bin/env bash
# test.sh <workdir> <part>
# Runs solution.py against the real input, isolates the requested part's
# answer, and emits on stderr:
#     runtime_ms: <float>
#     peak_kb: <int>
# and on stdout the "Part <part>: <answer>" line(s).
#
# Timing scope: wall-clock around the whole `python3 solution.py` process,
# matching the C/Go arms which time read+parse+solve for the part. Because
# solution.py prints BOTH parts in one run, we run it once and extract the
# requested part; the per-part runtime is therefore the full-program time.
# This is documented as a known cross-language nuance: Python is timed as a
# whole-program run, whereas C/Go time each part's solve call separately.
# For comparability the harness records both parts' runtime from the same
# single execution when the language is interpreted.
set -u
WORK="$1"
PART="$2"
SOL="$WORK/solution.py"

cd "$WORK" || { echo "cannot cd to workdir" >&2; exit 1; }

# Use /usr/bin/time -v for peak RSS if available, else fall back to no mem.
TIME_BIN=""
if [ -x /usr/bin/time ]; then TIME_BIN="/usr/bin/time -v"; fi

START_NS=$(date +%s%N)
if [ -n "$TIME_BIN" ]; then
  # capture both program output and /usr/bin/time's stderr separately
  $TIME_BIN python3 "$SOL" > /tmp/sol_out 2> /tmp/sol_time
  RC=$?
else
  python3 "$SOL" > /tmp/sol_out 2> /tmp/sol_err
  RC=$?
fi
END_NS=$(date +%s%N)

RUNTIME_MS=$(awk "BEGIN { printf \"%.2f\", ($END_NS - $START_NS) / 1000000 }")

# peak memory (KB) from /usr/bin/time -v "Maximum resident set size"
PEAK_KB=""
if [ -n "$TIME_BIN" ] && [ -f /tmp/sol_time ]; then
  PEAK_KB=$(grep "Maximum resident set size" /tmp/sol_time | grep -oE '[0-9]+' | head -1)
fi

# Emit the requested part's answer line(s) to stdout
grep -E "^Part[[:space:]]*${PART}[[:space:]]*:" /tmp/sol_out || true

# Emit measurements to stderr
echo "runtime_ms: $RUNTIME_MS" >&2
[ -n "$PEAK_KB" ] && echo "peak_kb: $PEAK_KB" >&2

exit 0
