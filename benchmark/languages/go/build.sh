#!/usr/bin/env bash
# build.sh <workdir>
# Builds the model's main.go into an executable named 'main' in the workdir.
# Emits for the harness to parse:
#     COMPILER=<go version>
#     OPTFLAG=go build (default)
#     WARNINGS=<count of 'go vet' findings>   (Go's nearest analog to -Wall)
# Exits non-zero if main.go is missing or fails to build.
#
# Note: Go has no -O level knob like C; the default optimizing build is the
# only mode used, recorded as OPTFLAG=go build (default) for consistency with
# the existing Go arm's measurement convention.
set -u
WORK="$1"
SRC="$WORK/main.go"
BIN="$WORK/main"

if ! command -v go >/dev/null 2>&1; then
  echo "WARNINGS=0"
  echo "go toolchain not found on PATH" >&2
  exit 1
fi

VER="$(go version 2>/dev/null)"
echo "COMPILER=$VER"
echo "OPTFLAG=go build (default)"

if [ ! -f "$SRC" ]; then
  echo "WARNINGS=0"
  echo "main.go not found" >&2
  exit 1
fi

cd "$WORK" || { echo "WARNINGS=0"; echo "cannot cd to workdir" >&2; exit 1; }

# A standalone main.go needs a module context for `go build`/`go vet` to work
# cleanly. Create a throwaway go.mod if none exists (stdlib-only, no deps).
if [ ! -f go.mod ]; then
  go mod init aocsolution >/dev/null 2>&1 || true
fi

# go vet for warning-equivalent signal (run before build; non-fatal).
VET_LOG="$(mktemp)"
go vet ./... > "$VET_LOG" 2>&1 || true
# Count vet findings: lines that look like file:line: diagnostics.
WARN=$(grep -cE '\.go:[0-9]+:' "$VET_LOG" 2>/dev/null || echo 0)
echo "WARNINGS=$WARN"
cat "$VET_LOG" >&2
rm -f "$VET_LOG"

# Build (this is the gating step — must succeed).
BUILD_LOG="$(mktemp)"
if go build -o "$BIN" main.go 2> "$BUILD_LOG"; then
  cat "$BUILD_LOG" >&2
  rm -f "$BUILD_LOG"
  exit 0
else
  cat "$BUILD_LOG" >&2
  rm -f "$BUILD_LOG"
  exit 1
fi
