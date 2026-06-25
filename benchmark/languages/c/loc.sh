#!/usr/bin/env bash
# loc.sh <workdir>  -> prints LOC=<n> for main.c
# Counts non-blank lines, excluding lines that are purely a // comment or a
# single-line /* ... */ comment or blank. (Approximate; matches the spirit of
# the existing C arm's LOC counting.)
set -u
WORK="$1"
SRC="$WORK/main.c"
if [ ! -f "$SRC" ]; then echo "LOC=0"; exit 0; fi
N=$(grep -vE '^[[:space:]]*$' "$SRC" \
    | grep -vE '^[[:space:]]*//' \
    | grep -vE '^[[:space:]]*/\*.*\*/[[:space:]]*$' \
    | wc -l | tr -d ' ')
echo "LOC=$N"
