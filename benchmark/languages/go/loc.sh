#!/usr/bin/env bash
# loc.sh <workdir>  -> prints LOC=<n> for main.go
# Counts non-blank lines, excluding lines that are purely a // comment.
set -u
WORK="$1"
SRC="$WORK/main.go"
if [ ! -f "$SRC" ]; then echo "LOC=0"; exit 0; fi
N=$(grep -vE '^[[:space:]]*$' "$SRC" \
    | grep -vE '^[[:space:]]*//' \
    | wc -l | tr -d ' ')
echo "LOC=$N"
