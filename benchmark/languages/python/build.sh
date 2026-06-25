#!/usr/bin/env bash
# build.sh <workdir>
# For an interpreted language there is no compile step, but we still:
#   - confirm the solution file exists and is syntactically valid
#   - emit COMPILER=, OPTFLAG=, WARNINGS= lines the harness parses
# Exit non-zero if the solution is missing or has a syntax error.
set -u
WORK="$1"
SOL="$WORK/solution.py"

PYVER="$(python3 --version 2>&1)"
echo "COMPILER=$PYVER"
echo "OPTFLAG=none (interpreted)"

if [ ! -f "$SOL" ]; then
  echo "WARNINGS=0"
  echo "solution.py not found" >&2
  exit 1
fi

# Syntax check (compile to bytecode without running).
if ! python3 -m py_compile "$SOL" 2> /tmp/pycompile_err; then
  echo "WARNINGS=0"
  cat /tmp/pycompile_err >&2
  exit 1
fi

# Optional lint warning count if pyflakes happens to be available; otherwise 0.
# (Kept null-safe: absence of the tool is not a build failure.)
if command -v pyflakes >/dev/null 2>&1; then
  WARN=$(pyflakes "$SOL" 2>/dev/null | wc -l | tr -d ' ')
else
  WARN=0
fi
echo "WARNINGS=$WARN"
exit 0
