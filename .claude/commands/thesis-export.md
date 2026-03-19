---
description: Generate thesis-ready analysis from the research log
allowed-tools: Read, Write, Bash, Grep, Glob
---

# Generate Thesis Analysis

Read all research data and produce a structured analysis document.

## Data Sources
- `docs/thesis-log.jsonl` — structured results for every attempt
- `docs/observations.md` — qualitative notes
- `day*/main.go` — the actual solutions (for complexity analysis)
- `day*/main_test.go` — test coverage

## Generate `docs/thesis-analysis.md` with these sections:

### 1. Quantitative Results
- Table: Day | Part 1 | Part 2 | Attempts | Failure Category | Duration
- Success rates by part
- Average attempts needed
- Distribution of failure categories (as a frequency table)

### 2. Failure Taxonomy
- Group failures by category
- For each category: count, which days, brief explanation of what went wrong
- Identify if certain problem TYPES (parsing, graph, DP, simulation, math) correlate with failures

### 3. Difficulty Progression
- Did failures increase with day number?
- Were there clusters of difficulty?
- Compare early days (1-8) vs mid (9-16) vs late (17-25)

### 4. Retry Analysis
- Which days needed retries?
- Did the retry succeed?
- What changed between attempts?

### 5. Code Quality Metrics
- Lines of code per solution
- Whether utils/ was reused
- Test coverage (tests exist? pass?)

### 6. Key Observations
- Pull the most interesting findings from observations.md
- Highlight surprising successes and surprising failures

Write this as clean markdown that can be copy-pasted into a LaTeX or Word thesis document.
