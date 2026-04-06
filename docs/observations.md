# Observations & Analysis

## Running Notes

_Add observations here as you work through the days. This feeds directly into thesis writing._

### 2026-03-19 — Day 1: Secret Entrance

Day 1 was solved completely on the first attempt for both parts (~8 min total). Part 1 was a trivial circular dial simulation using modular arithmetic. Part 2 required counting every click through 0 mid-rotation, not just final positions — this demanded a closed-form calculation (first crossing at step `pos` or `100-pos`, then every 100 steps) rather than brute-force stepping through each click. Claude Code derived this formula correctly without hints. No utils were needed. This day serves as a clean baseline: simple simulation, no algorithmic complexity, zero failures.

### 2026-04-06 — Day 2: Gift Shop

Day 2 required finding "repeated-digit" numbers within given ranges and summing them. An invalid ID is a number whose digit string is some sequence of length k repeated m times (m≥2 for Part 2, m=2 for Part 1). The key mathematical insight: such a number equals `n * (1 + 10^k + 10^(2k) + … + 10^((m-1)k))` for a k-digit prefix `n`. Rather than scanning every integer in each range (which could be enormous), the solution generates all candidate repeated numbers up to the max range value, then checks membership. This kept the solution fast regardless of range size. Part 2 required extending the inner loop over repetition counts and deduplicating via a set (since e.g. `111111` is both "11" repeated 3 times and "111" repeated 2 times). `MustParseInt64` was added to utils to handle large 64-bit inputs. Both parts solved on first attempt with clean tests.

### 2026-04-06 — Day 3: Lobby

Day 3 asked for the largest k-digit subsequence (preserving order) from each line of digits, summed across all lines. Part 1 used k=2 with a brute-force O(n²) pair search. Part 2 generalised to k=12 requiring the classic greedy algorithm: at each of the k picks, find the maximum digit in the window `[prev+1, n-k+i]` (which leaves exactly enough digits for the remaining picks). One failure occurred due to a tie-breaking bug — `>=` picked the rightmost occurrence of the max digit, but the correct behaviour is leftmost (`>`) to preserve future flexibility. After fixing, both parts used a single `maxKJoltage(s, k)` function. Interesting that a subtle `>` vs `>=` distinction caused the only wrong answer.

## Patterns Noticed

## Failure Analysis

## Interesting Behaviors
