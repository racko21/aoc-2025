#!/usr/bin/env bash
# loc.sh <workdir>  -> prints LOC=<n> (non-blank, non-comment lines of solution.py)
set -u
WORK="$1"
SOL="$WORK/solution.py"
if [ ! -f "$SOL" ]; then echo "LOC=0"; exit 0; fi
N=$(grep -cvE '^[[:space:]]*(#|$)' "$SOL")
echo "LOC=$N"
