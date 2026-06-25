#!/usr/bin/env bash
# build.sh <workdir>
# Compiles the model's main.c at -O2 with the standard warning flags, into an
# executable named 'main' in the workdir. Emits, for the harness to parse:
#     COMPILER=<gcc/cc version line>
#     OPTFLAG=-O2
#     WARNINGS=<count of compiler 'warning:' lines>
# Exits non-zero if main.c is missing or fails to compile (errors).
set -u
WORK="$1"
SRC="$WORK/main.c"
BIN="$WORK/main"

# Pick a compiler: prefer gcc, fall back to cc/clang.
CC=""
for c in gcc cc clang; do
  if command -v "$c" >/dev/null 2>&1; then CC="$c"; break; fi
done
if [ -z "$CC" ]; then
  echo "WARNINGS=0"
  echo "no C compiler found (tried gcc, cc, clang)" >&2
  exit 1
fi

VER="$("$CC" --version 2>/dev/null | head -1)"
echo "COMPILER=$VER"
echo "OPTFLAG=-O2"

if [ ! -f "$SRC" ]; then
  echo "WARNINGS=0"
  echo "main.c not found" >&2
  exit 1
fi

# Compile, capturing diagnostics. Link -lm so math-using solutions build.
BUILD_LOG="$(mktemp)"
if "$CC" -std=c17 -Wall -Wextra -Wpedantic -O2 -o "$BIN" "$SRC" -lm 2> "$BUILD_LOG"; then
  WARN=$(grep -c "warning:" "$BUILD_LOG" 2>/dev/null || echo 0)
  echo "WARNINGS=$WARN"
  # surface warnings on stderr too (visible in transcript / audit)
  cat "$BUILD_LOG" >&2
  rm -f "$BUILD_LOG"
  exit 0
else
  WARN=$(grep -c "warning:" "$BUILD_LOG" 2>/dev/null || echo 0)
  echo "WARNINGS=$WARN"
  cat "$BUILD_LOG" >&2
  rm -f "$BUILD_LOG"
  exit 1
fi
