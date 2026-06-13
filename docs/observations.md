# Observations & Analysis

## Running Notes

_Add observations here as you work through the days. This feeds directly into thesis writing._

### 2026-03-19 — Day 1: Secret Entrance

Day 1 was solved completely on the first attempt for both parts (~8 min total). Part 1 was a trivial circular dial simulation using modular arithmetic. Part 2 required counting every click through 0 mid-rotation, not just final positions — this demanded a closed-form calculation (first crossing at step `pos` or `100-pos`, then every 100 steps) rather than brute-force stepping through each click. Claude Code derived this formula correctly without hints. No utils were needed. This day serves as a clean baseline: simple simulation, no algorithmic complexity, zero failures.

### 2026-04-06 — Day 2: Gift Shop

Day 2 required finding "repeated-digit" numbers within given ranges and summing them. An invalid ID is a number whose digit string is some sequence of length k repeated m times (m≥2 for Part 2, m=2 for Part 1). The key mathematical insight: such a number equals `n * (1 + 10^k + 10^(2k) + … + 10^((m-1)k))` for a k-digit prefix `n`. Rather than scanning every integer in each range (which could be enormous), the solution generates all candidate repeated numbers up to the max range value, then checks membership. This kept the solution fast regardless of range size. Part 2 required extending the inner loop over repetition counts and deduplicating via a set (since e.g. `111111` is both "11" repeated 3 times and "111" repeated 2 times). `MustParseInt64` was added to utils to handle large 64-bit inputs. Both parts solved on first attempt with clean tests.

### 2026-04-06 — Day 3: Lobby

Day 3 asked for the largest k-digit subsequence (preserving order) from each line of digits, summed across all lines. Part 1 used k=2 with a brute-force O(n²) pair search. Part 2 generalised to k=12 requiring the classic greedy algorithm: at each of the k picks, find the maximum digit in the window `[prev+1, n-k+i]` (which leaves exactly enough digits for the remaining picks). One failure occurred due to a tie-breaking bug — `>=` picked the rightmost occurrence of the max digit, but the correct behaviour is leftmost (`>`) to preserve future flexibility. After fixing, both parts used a single `maxKJoltage(s, k)` function. Interesting that a subtle `>` vs `>=` distinction caused the only wrong answer.

### 2026-06-04 — Day 4: Printing Department

Day 4 was solved cleanly on the first attempt for both parts (~10 min total). Part 1 was a straightforward 8-directional neighbor count: a roll of paper is accessible if it has fewer than 4 `@` neighbours. Part 2 extended this to an iterative simulation — each round, all currently accessible rolls are removed simultaneously (not sequentially), and the process repeats until no accessible rolls remain. The critical design choice was batching removals per round rather than removing one-at-a-time; the problem statement confirmed this by showing "Remove N rolls of paper" in bulk steps. Claude Code represented the mutable grid as `[][]byte` for Part 2 while keeping the original `[]string` representation for Part 1 — a clean separation that avoided any aliasing issues. No algorithmic difficulty; the main complexity was recognising the batched-round structure from the problem wording. Zero failures, both parts correct first attempt.

### 2026-06-04 — Day 5: Cafeteria

Day 5 split cleanly into two independent sub-problems over the same input. Part 1 was simple range membership: for each available ID, check if it falls in any fresh range (linear scan, no optimisation needed). Part 2 discarded the available IDs entirely and asked for the total integer count covered by the union of all ranges — solved with the classic sort-and-merge interval algorithm. The twist was recognising that Part 2 ignores the second section of the input. Claude Code identified this immediately from the problem wording and didn't attempt to reuse Part 1's logic inappropriately. Clean, minimal solution with no failures.

### 2026-06-04 — Day 6: Trash Compactor

Day 6 was the most parsing-intensive problem so far. The input is a single wide grid (3727 characters per row, 5 rows) where numbers and operators are laid out in vertical column bands separated by all-space columns. Part 1: each row within a band contributes one number; trim whitespace and parse. Part 2: the reading direction inverts — each character column within a band encodes one number (non-space chars top-to-bottom as digits), and bands are read right-to-left. The code was cleanly refactored with a shared `solve()` function taking a `bandSolver` argument, so the two interpretations required only two different leaf functions. No failures; both parts correct first attempt. Notable: this is the first problem where algorithmic complexity was dominated by parsing structure rather than computation.

### 2026-06-04 — Day 7: Laboratories

Day 7 was a beam-propagation simulation. Part 1 counted beam splits as a tachyon beam descends row-by-row, hitting '^' splitters that fork it left and right; beams at the same column merge (set semantics). Part 2 changed the interpretation to counting distinct independent timelines — beams at the same column accumulate multiplicatively rather than collapsing. The only code change was replacing `map[int]bool` with `map[int]int` and propagating counts instead of presence. Solved in ~16 minutes on first attempt; the set→multiset generalisation was identified immediately. The structural parallelism between Part 1 and Part 2 is a clean example of a problem designed so Part 2 requires a single, well-understood abstraction lift.

### 2026-06-04 — Day 8: Playground

Day 8 was a union-find / Kruskal problem over 3D junction box coordinates. Part 1 connected the 1000 closest pairs by Euclidean distance and returned the product of the three largest component sizes. Part 2 continued connecting until all boxes formed a single component, returning the product of the X-coordinates of the two nodes whose edge caused the final merge. The key challenge was reading these as two conceptually distinct questions over the same process: Part 1 inspects the state at a fixed step count, Part 2 inspects the terminal condition. Duration ~57 minutes (estimated from timestamps) suggests some iteration; the two parts share union-find cleanly. No algorithmic surprises — Kruskal/UF is a well-understood pattern — but the "last merging edge" framing of Part 2 required careful attention.

### 2026-06-04 — Day 9: Movie Theater

Day 9 had a sharp complexity jump between parts. Part 1 was trivial: O(n²) brute-force over all pairs of red tiles as opposite rectangle corners. Part 2 added the constraint that the rectangle must lie entirely within a rectilinear polygon defined by the tile sequence. The solution used coordinate compression with parity indexing (tile coords mapped to odd indices; gap cells get even indices), traced the polygon boundary on the compressed grid, BFS flood-filled to identify exterior cells, then built a 2D prefix sum for O(1) rectangle-interior queries. Any pair of tiles with zero exterior cells in their bounding rectangle is valid. Duration ~69 minutes, estimated 3 attempts, reflecting the non-obvious step of parity-aware coordinate compression — a technique needed to correctly represent both tile-boundaries and gap-regions without conflating them. This is one of the more technically demanding problems in the set for its size.

### 2026-06-04 — Day 10: Factory

Day 10 split into two structurally distinct optimisation problems sharing the same input format. Part 1 (answer: 411) was minimum-weight solution over GF(2) — toggle buttons to match a target light pattern. This is equivalent to minimum-weight codeword finding, solved here with BFS over 2^N states (feasible because the number of lights per machine is small). Part 2 (answer: 16048) changed the semantics completely: buttons now increment integer counters, and the goal is to reach specified joltage targets with fewest total presses. This is a non-negative integer LP: min Σx_j subject to Ax=b, x≥0.

The most significant aspect of this day was implementing a Big-M simplex solver from scratch in Go (~80 lines). The problem required recognising the LP structure, setting up the Big-M objective row with the correct sign convention (the RHS of row 0 stores −z, not +z — a subtle point that causes silent wrong answers if inverted), and using Bland's rule for tie-breaking to guarantee termination without cycling. The LP relaxation gave exact integer solutions for all machines in the real input, confirming the AoC instances have integer LP vertices despite the general problem being NP-hard. No branch-and-bound was needed.

Notable for thesis: this is the first day requiring a non-trivial mathematical algorithm (LP/ILP) that is typically associated with operations research rather than competitive programming. Part 2 required four attempts and extensive debugging of the numerical LP implementation. Three distinct bugs were found sequentially: (1) incorrect rounding of fractional LP values (math.Round instead of B&B); (2) upper-bound constraints added as equalities instead of inequalities with slack variables, causing wrong sub-problems; (3) degenerate Phase-1 artificials corrupting Phase 2 by remaining in the basis and causing the simplex to produce "integer" solutions that violated the equality constraints. The final solution drives out degenerate artificials before Phase 2 and verifies all candidate integer solutions against Ay=b. This day is the most algorithmically complex of the set, requiring deep knowledge of LP theory and numerical methods. Claude Code correctly diagnosed each bug but required user feedback (wrong answer from the AoC grader) to iterate — it could not self-detect that its solutions were wrong without external oracle feedback. The final code is ~320 lines, significantly longer than typical AoC solutions.

### 2026-06-11 — Day 11: Reactor

Day 11 was a straightforward directed-graph path-counting problem — among the cleanest of the set. Part 1: count all paths from "you" to "out" in a DAG parsed from an adjacency-list. Memoised DFS reduces this to O(N+E) time — a textbook competitive-programming pattern that Claude Code applied immediately without any missteps. Part 2 extended the requirement: count only paths (now from "svr") that visit both "dac" and "fft". The key insight is to lift the DFS state from a single node label to a 3-tuple (node, haveDac, haveFft), giving a 4× expansion of the memo table but no change in algorithmic complexity. Both parts solved in a single attempt with zero wrong answers.

Notable for thesis: this is the first day where Part 2 was a clean, principled generalisation of Part 1's algorithm rather than a completely different approach. The structural pattern — expanding memoisation state to track auxiliary conditions — is a reusable technique. Claude Code identified it immediately and produced minimal, correct code without hallucination or trial-and-error. The contrast with Day 10 (four attempts, 90 minutes, bespoke LP solver) highlights how problem type dominates difficulty: graph/DP problems are well within the training distribution, while novel numerical algorithms are not.

### 2026-06-13 — Day 12: Christmas Tree Farm

Day 12 was a polyomino packing problem: given a set of shapes (each within a 3×3 bounding box, with '.' cells transparent) and a list of W×H regions, determine how many regions can fit all required present copies without '#'-cell overlaps. Shapes can be rotated and flipped (up to 8 orientations each).

The naive approach — backtracking over all placements — was implemented first and passed the example in 0.73s for 3 regions. It would be far too slow for 1000 real-input regions of up to 50×50 with 200+ presents each. The key insight came from input analysis: (1) every infeasible region is caught by the simple cell count check (total '#' cells > W×H); (2) every region that passes the cell count check also satisfies presents ≤ ⌊W/3⌋×⌊H/3⌋ non-overlapping aligned 3×3 slots — meaning all such regions are trivially packable by assigning each present to a unique aligned block. The final algorithm runs in ~73ms for all 1000 regions.

Notable for thesis: this is the clearest example so far of mathematical input analysis revealing that a computationally hard general problem (bin packing / exact cover) reduces to a trivial condition for this specific dataset. Claude Code initially went deep into algorithmic complexity (DLX, profile DP, coloring arguments) before stepping back and testing the "aligned slots" hypothesis empirically. The backtracking fallback was kept for correctness on the example (which includes a region where the cell count passes but packing is genuinely infeasible — requiring actual search). The final design uses a layered approach: fast O(1) checks first, expensive search only as fallback. Part 2 was not yet available.

## Patterns Noticed

## Failure Analysis

## Interesting Behaviors
