# Complete Setup Guide: Benchmarking Claude Code with Advent of Code 2025

## Overview

This guide covers the full toolchain for your bachelor thesis: how to configure Claude Code and Claude Chat to maximize AI leverage, automate repetitive work, maintain context across months, and systematically collect thesis-ready research data.

---

## Part 1: Repository Setup

### What I've prepared for you

The project scaffold contains 14 files across 4 categories:

**Core project files:**
- `CLAUDE.md` — Project constitution that Claude Code reads every session
- `go.mod` — Go module definition
- `.gitignore` — Ignores `input.txt` files (AoC policy: don't share puzzle inputs)
- `utils/parse.go` — Input parsing, math helpers (Atoi, GCD, LCM, Abs, Min, Max)
- `utils/grid.go` — 2D grid types and operations (Point, Dir4, Dir8, BFS helpers)

**Claude Code configuration (`.claude/` directory):**
- `settings.json` — Permissions and hooks (auto-runs `gofmt` on every Go file write)
- `commands/solve.md` — `/solve <day>` command: full pipeline from setup to solution
- `commands/log.md` — `/log <day>` command: document results to JSONL
- `commands/retry.md` — `/retry <day>` command: re-attempt with fresh approach
- `commands/status.md` — `/status` command: progress dashboard across all 25 days
- `commands/thesis-export.md` — `/thesis-export` command: generate thesis-ready analysis
- `skills/aoc-patterns/SKILL.md` — Algorithm pattern reference (BFS, cycle detection, DP)
- `skills/research-log/SKILL.md` — Structured logging format for thesis data

**Research data files (`docs/`):**
- `docs/thesis-log.jsonl` — Machine-readable log of every attempt (starts empty)
- `docs/observations.md` — Qualitative notes and analysis (starts with template)

### Installation steps

```bash
# 1. Clone your empty repo
git clone https://github.com/<you>/aoc-2025.git
cd aoc-2025

# 2. Copy all the scaffold files into it
#    (download the zip I've provided, or copy file by file)

# 3. Update go.mod with your actual module path
sed -i 's|github.com/yourusername/aoc-2025|github.com/<you>/aoc-2025|' go.mod

# 4. Verify Go setup
go vet ./utils/...

# 5. Commit the scaffold
git add -A
git commit -m "chore: initial project scaffold for AoC 2025 benchmark"
git push

# 6. Open Claude Code in the repo
cd aoc-2025
claude
```

On first launch, Claude Code reads `CLAUDE.md` and knows: the language is Go, every day gets its own folder, tests come first, and results get logged to JSONL.

---

## Part 2: How `CLAUDE.md` Works and Why It Matters

`CLAUDE.md` is loaded into Claude Code's system prompt at the start of every session. It acts as persistent memory — you never have to re-explain the project.

### Design principles applied

1. **Under 100 lines** — Research shows that as instruction count grows, adherence drops uniformly across all instructions, not just the new ones. The file is ~80 lines.

2. **Concrete, not abstract** — Instead of "use consistent folder structure," it specifies exact paths: `dayXX/main.go`, `dayXX/main_test.go`, etc.

3. **Includes verification commands** — `go test ./...`, `go run .`, `gofmt -w` — so Claude always knows how to check its own work.

4. **Compaction instructions** — When context fills up and Claude compacts the conversation, it preserves the current day, failing test output, and completion status. This prevents losing critical state mid-session.

5. **No linting rules** — That's what `gofmt` and hooks are for. Don't use CLAUDE.md for things tools can enforce.

### Evolving it over time

As you work through the 25 days, you'll discover patterns. Add them:

```markdown
## Learned Patterns
- Days with grid problems: always check for negative coordinates
- Days with "after N billion steps": always try cycle detection first
- Part 2 often requires refactoring Part 1 — keep functions small
```

But prune aggressively. If Claude already does something without being told, remove that instruction.

---

## Part 3: Custom Slash Commands — Your Workflow Automation

### The daily workflow

```
You:     Download input.txt from AoC, drop it in day05/input.txt
You:     Copy problem text, paste into Claude Code
Claude:  (saves to day05/PROBLEM.md, extracts example.txt)
You:     /solve 5
Claude:  (creates folder, writes tests, solves Part 1, solves Part 2, logs results)
You:     /log 5
Claude:  (appends structured JSONL entry, updates observations.md)
```

That's it. Two manual steps (download input, paste problem), then slash commands handle the rest.

### Command reference

| Command | Purpose | When to use |
|---|---|---|
| `/solve <day>` | Complete solve pipeline | Starting a new day |
| `/log <day>` | Record results to thesis log | After solving (success or failure) |
| `/retry <day>` | Fresh attempt preserving old code | After a failure you want to re-try |
| `/status` | Progress dashboard | Checking overall progress |
| `/thesis-export` | Generate analysis document | When writing thesis chapters |

### How commands work under the hood

Each command is a Markdown file in `.claude/commands/`. The filename becomes the command name. `$1` is replaced with the first argument you pass. Claude Code reads the file content as a prompt and follows the instructions.

You can create new commands at any time — just add a `.md` file to `.claude/commands/`.

Ideas for additional commands:
- `/compare <day1> <day2>` — Compare approaches between two days
- `/optimize <day>` — Try to improve performance of an existing solution
- `/explain <day>` — Generate a detailed explanation of the solution for the thesis

---

## Part 4: Skills — Reusable Knowledge

### Difference between commands and skills

**Commands** = saved prompts you invoke manually with `/command-name`. Think of them as macros.

**Skills** = knowledge packages that Claude loads automatically when relevant. Think of them as reference manuals.

The `aoc-patterns` skill is loaded automatically whenever Claude is working on a puzzle and encounters a pattern it recognizes (BFS, cycle detection, etc.). You don't invoke it — Claude decides to reference it.

The `research-log` skill ensures consistent logging format. Whenever Claude sees "log this" or "document the result," it pulls in the exact JSONL schema and failure categories.

### Adding skills over time

As you discover recurring problem patterns, add them as skills:

```
.claude/skills/
├── aoc-patterns/SKILL.md      # Algorithm reference
├── research-log/SKILL.md      # Logging format
├── go-idioms/SKILL.md         # Go-specific patterns you find useful
└── debug-strategies/SKILL.md  # Debugging approaches that worked
```

---

## Part 5: Hooks — Automatic Quality Gates

The `settings.json` includes one hook:

```json
{
  "hooks": {
    "PostToolUse": [
      {
        "matcher": "Write(*.go)",
        "hooks": [
          {
            "type": "command",
            "command": "gofmt -w $CLAUDE_FILE_PATH",
            "timeout": 5
          }
        ]
      }
    ]
  }
}
```

This runs `gofmt` on every `.go` file Claude writes — automatically, with zero exceptions. Unlike CLAUDE.md instructions (which are advisory), hooks are deterministic.

### Hooks to consider adding later

```json
{
  "PostToolUse": [
    {
      "matcher": "Write(*.go)",
      "hooks": [
        { "type": "command", "command": "gofmt -w $CLAUDE_FILE_PATH", "timeout": 5 },
        { "type": "command", "command": "go vet ./...", "timeout": 10 }
      ]
    }
  ]
}
```

You can also add a `PreToolUse` hook to prevent writes to protected files:

```json
{
  "PreToolUse": [
    {
      "matcher": "Write(docs/thesis-log.jsonl)",
      "hooks": [
        {
          "type": "command",
          "command": "echo 'Reminder: only append to thesis-log.jsonl, never overwrite'",
          "timeout": 3
        }
      ]
    }
  ]
}
```

---

## Part 6: Context Persistence Across Sessions

### The problem
Claude Code has no memory between sessions. Every time you open it, it starts fresh (except for CLAUDE.md which loads automatically).

### The solution: layered context

| Layer | What it stores | How it persists |
|---|---|---|
| `CLAUDE.md` | Project rules, structure, workflow | Auto-loaded every session |
| `docs/thesis-log.jsonl` | Structured results for every attempt | File on disk, read on demand |
| `docs/observations.md` | Qualitative notes | File on disk, read on demand |
| Git history | Every code change | `git log --oneline` |
| Slash commands | Workflow procedures | Files in `.claude/commands/` |

### Resuming after time away

When you return after days or weeks, start with:

```
> Read docs/thesis-log.jsonl and docs/observations.md, then tell me where I left off.
```

Claude reads the files, sees which days are done, which failed, and gives you a status update. Then continue:

```
> /solve 14
```

### The `/compact` command

During long sessions, Claude's context window fills up. Use `/compact` to summarize older context. The CLAUDE.md includes instructions to preserve critical state during compaction:

> "When compacting, always preserve the full list of modified files and any test commands."

You can also manually compact with focus:

```
/compact Focus on day 12 Part 2 — keep the failing test output and the current algorithm
```

---

## Part 7: Using Claude Chat (This Interface) as a Complement

Claude Code and Claude Chat serve different roles:

### Claude Code = the executor
- Solves puzzles
- Writes and runs code
- Manages files and git
- Follows slash commands

### Claude Chat = the analyst
- Analyze failure patterns across days
- Draft thesis sections (introduction, methodology, results, discussion)
- Create data visualizations from your JSONL log
- Discuss Claude Code's behavior as a research subject
- Brainstorm thesis arguments

### Practical workflow

1. **After each week of solving:** Copy `thesis-log.jsonl` here and ask:
   *"Here are my results from days 1-7. What patterns do you see? Which failure categories dominate? What does this suggest about LLM capabilities?"*

2. **When writing the thesis:** Share your observations and ask:
   *"Help me structure a Results section. Here's my data. I need to argue that [X]."*

3. **When debugging Claude Code's behavior:** Paste the conversation transcript and ask:
   *"Claude Code failed on Day 18. Here's what happened. Why do you think it misread the problem? What category of failure is this?"*

4. **For visualizations:** Share data here and ask for charts — success rates over time, failure category distributions, attempts per day, etc.

---

## Part 8: On Web Scraping AoC (Don't)

Advent of Code's creator (Eric Wastl) has explicitly asked participants not to automate interactions with the site. This matters especially for a thesis, which is a public academic document. The manual steps are minimal:

1. Open the day's page → Copy problem text → Paste into Claude Code
2. Click "get your puzzle input" → Save as `dayXX/input.txt`
3. After solving → Type the answer into the AoC website
4. Record pass/fail

This takes ~60 seconds per day and keeps your thesis on solid ethical ground.

**What you CAN automate:** everything that happens after the input is in your repo. That's what the slash commands do.

---

## Part 9: Thesis Documentation Strategy

### Three-layer data collection

**Layer 1: Structured (automatic)**
Every `/log` command appends a JSON line with day, correctness, attempts, failure category, algorithm, problem type, duration, and model used. This becomes your quantitative dataset.

**Layer 2: Qualitative (semi-automatic)**
The `research-log` skill prompts Claude to add observations to `observations.md`. You supplement with your own notes about Claude Code's behavior.

**Layer 3: Code artifacts (automatic)**
Git history captures every solution version. You can diff V1 vs V2 of a retry to show exactly what changed.

### Suggested thesis structure

1. **Introduction** — AI-assisted coding is growing; how capable are current LLMs at algorithmic problem-solving?
2. **Background** — Advent of Code as a benchmark, Claude Code as the tool, Go as the language
3. **Methodology** — Your setup (this guide), the solve/log/retry workflow, failure taxonomy
4. **Results** — Quantitative (success rates, failure distributions) and qualitative (interesting behaviors)
5. **Discussion** — Where Claude Code excels (parsing, standard algorithms), where it fails (novel reasoning, edge cases), what this means for AI-assisted development
6. **Conclusion** — Current capabilities and limitations, recommendations for using AI coding tools

### Using `/thesis-export`

Run this command in Claude Code after completing a batch of days. It reads all your data and generates a structured analysis document with tables, statistics, and patterns — ready to paste into your thesis.

---

## Part 10: Experimental Methodology Tips

### For scientific rigor

- **Don't help Claude Code** (at first). Let it attempt each day fully autonomously. Record the unassisted result. Only then try a retry with hints.
- **Record the model used.** Different models (Opus vs Sonnet) will perform differently. Note which one was used for each attempt.
- **Time the attempts.** Even a rough estimate helps analyze whether harder problems take longer.
- **Use the same prompt format.** The `/solve` command ensures consistency. Don't ad-lib extra hints unless you're recording that as a separate experimental condition.

### Experimental conditions to consider

| Condition | Description |
|---|---|
| Baseline | `/solve <day>` with no hints, no human intervention |
| With hints | After baseline fails, provide a hint about the algorithm and retry |
| With examples | Provide additional test cases beyond the problem's example |
| Different model | Run the same day with Opus and Sonnet, compare |

Each condition gets its own entry in `thesis-log.jsonl`. The `notes` field captures what was different.

### Statistical considerations

With 25 days × 2 parts = 50 data points at baseline, you have enough for descriptive statistics but not inferential statistics with strong power. Focus on:
- Success rates (overall, by part, by problem type)
- Failure category distributions
- Trends over difficulty progression
- Qualitative case studies of interesting failures

---

## Quick Reference Card

```
# Daily workflow
1. Download input.txt, paste problem text
2. /solve <day>
3. /log <day>
4. git add -A && git commit -m "day<XX>: <result>"

# Weekly workflow
5. /status                    (check progress)
6. Paste thesis-log.jsonl into Claude Chat for analysis

# When stuck
7. /retry <day>              (fresh approach)

# End of semester
8. /thesis-export            (generate analysis)
9. Write thesis in Claude Chat with the exported data

# Context management
- New session: "Read docs/thesis-log.jsonl and observations.md"
- Long session: /compact Focus on <current task>
- New topic: /clear
```
