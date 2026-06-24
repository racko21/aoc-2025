#!/usr/bin/env bash
# PreToolUse hook: while a C-arm session is active (marker file present),
# block any tool call that could expose a Go solution file (*.go anywhere
# outside c/) to a C-arm session. This exists because slash-command
# frontmatter `allowed-tools` is additive-only in Claude Code's permission
# model and cannot override the global bare Read/Write grants in
# settings.json — only a hook intercepts a tool call before it executes and
# can unconditionally deny it. See c/CLAUDE.md isolation policy.
#
# Fails closed: any internal error (missing jq, malformed hook input, etc.)
# is treated as a denial, not a pass-through.
set -uo pipefail

PROJECT_DIR="${CLAUDE_PROJECT_DIR:-$PWD}"
MARKER="$PROJECT_DIR/.claude/.c-arm-session"
DENY_LOG="$PROJECT_DIR/docs/isolation-denials.log"

TOOL=""
TARGET=""

deny() {
  local reason="$1"
  {
    printf '%s BLOCKED tool=%s target=%q reason=%s\n' \
      "$(date -u +%FT%TZ)" "$TOOL" "$TARGET" "$reason"
  } >>"$DENY_LOG" 2>/dev/null || true
  echo "Blocked: a C-arm session is active — access to Go solution files (*.go anywhere outside c/) is forbidden to prevent porting Go solutions into the C arm. Solve from scratch using only c/dayNN/PROBLEM.md and c/dayNN/example.txt. ($reason)" >&2
  exit 2
}

# No marker -> not a C-arm session, nothing to enforce.
[ -f "$MARKER" ] || exit 0

INPUT="$(cat)" || deny "failed to read hook input from stdin"

TOOL="$(jq -r '.tool_name // empty' <<<"$INPUT" 2>/dev/null)" || deny "jq failed parsing tool_name"
[ -n "$TOOL" ] || deny "empty tool_name in hook input"

case "$TOOL" in
  Read|Edit)
    TARGET="$(jq -r '.tool_input.file_path // empty' <<<"$INPUT" 2>/dev/null)" || deny "jq failed parsing file_path"
    if echo "$TARGET" | grep -qE '\.go\b'; then
      deny "target references a .go file"
    fi
    ;;
  Grep|Glob)
    GPATH="$(jq -r '.tool_input.path // empty' <<<"$INPUT" 2>/dev/null)" || deny "jq failed parsing path"
    GPATTERN="$(jq -r '.tool_input.pattern // empty' <<<"$INPUT" 2>/dev/null)" || deny "jq failed parsing pattern"
    TARGET="path=$GPATH pattern=$GPATTERN"
    if echo "$GPATTERN" | grep -qE '\.go\b'; then
      deny "pattern references a .go file"
    fi
    if [ -z "$GPATH" ]; then
      deny "no path scope given — would default to the full repo, including Go solutions"
    fi
    case "$GPATH" in
      c/*|./c/*|docs/*|./docs/*) ;; # explicitly scoped to a safe zone
      *) deny "path '$GPATH' is outside the c/ and docs/ isolation scope" ;;
    esac
    ;;
  Bash)
    TARGET="$(jq -r '.tool_input.command // empty' <<<"$INPUT" 2>/dev/null)" || deny "jq failed parsing command"
    if echo "$TARGET" | grep -qE '\.go\b'; then
      deny "command references a .go file"
    fi
    if echo "$TARGET" | grep -qE '\b(grep[[:space:]]+-[A-Za-z]*[rR]|find|rg|ack)\b' \
       && ! echo "$TARGET" | grep -qE '(^|[[:space:]])\.?/?c/'; then
      deny "recursive search command not explicitly scoped to c/ — could expose Go solutions"
    fi
    ;;
  *)
    exit 0
    ;;
esac

exit 0
