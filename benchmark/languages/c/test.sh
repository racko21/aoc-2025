#!/usr/bin/env bash
# test.sh <workdir> <part>
# Runs the compiled ./main with the part argument against the real input,
# emitting on stdout the "Part <part>: <answer>" line, and on stderr:
#     runtime_ms: <float>
#     peak_kb: <int>
#
# Timing scope: wall-clock around the single-part invocation (./main <part>),
# which performs read+parse+solve for that part — matching the C arm's
# per-part timing convention.
set -u
WORK="$1"
PART="$2"
BIN="$WORK/main"

cd "$WORK" || { echo "cannot cd to workdir" >&2; exit 1; }

if [ ! -x "$BIN" ]; then
  echo "main binary not built" >&2
  exit 1
fi

TIME_BIN=""
if [ -x /usr/bin/time ]; then TIME_BIN="/usr/bin/time -v"; fi

START_NS=$(date +%s%N)
if [ -n "$TIME_BIN" ]; then
  $TIME_BIN ./main "$PART" > /tmp/c_sol_out 2> /tmp/c_sol_time
  RC=$?
else
  ./main "$PART" > /tmp/c_sol_out 2> /tmp/c_sol_err
  RC=$?
fi
END_NS=$(date +%s%N)

RUNTIME_MS=$(awk "BEGIN { printf \"%.2f\", ($END_NS - $START_NS) / 1000000 }")

PEAK_KB=""
if [ -n "$TIME_BIN" ] && [ -f /tmp/c_sol_time ]; then
  PEAK_KB=$(grep "Maximum resident set size" /tmp/c_sol_time | grep -oE '[0-9]+' | head -1)
fi

# Emit the requested part's answer line(s)
grep -E "^Part[[:space:]]*${PART}[[:space:]]*:" /tmp/c_sol_out || true

echo "runtime_ms: $RUNTIME_MS" >&2
[ -n "$PEAK_KB" ] && echo "peak_kb: $PEAK_KB" >&2

exit 0
