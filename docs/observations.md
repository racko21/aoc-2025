# Observations & Analysis

## Running Notes

_Add observations here as you work through the days. This feeds directly into thesis writing._

### 2026-03-19 — Day 1: Secret Entrance

Day 1 was solved completely on the first attempt for both parts (~8 min total). Part 1 was a trivial circular dial simulation using modular arithmetic. Part 2 required counting every click through 0 mid-rotation, not just final positions — this demanded a closed-form calculation (first crossing at step `pos` or `100-pos`, then every 100 steps) rather than brute-force stepping through each click. Claude Code derived this formula correctly without hints. No utils were needed. This day serves as a clean baseline: simple simulation, no algorithmic complexity, zero failures.

## Patterns Noticed

## Failure Analysis

## Interesting Behaviors
