# Day 10: Factory

Each machine line: `[target] (btn)... {joltage (ignored)}`.
Lights start all-off. Buttons toggle specified lights (XOR). Find minimum total presses across all machines to reach each target.

## Part 1
Minimum total button presses to configure all machines. Each machine is an independent GF(2) system.

### Example
```
[.##.] (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}
[...#.] (0,2,3,4) (2,3) (0,4) (0,1,2) (1,2,3,4) {7,5,12,7,2}
[.###.#] (0,1,2,3,4) (0,3,4) (0,1,2,4,5) (1,2) {10,11,11,5,10,5}
```
Expected: 7 (2+3+2)

## Part 2
Lever switches buttons to joltage mode: each press of a button increases (not toggles) every counter it lists by 1. Counters start at 0; reach the exact target vector with fewest total presses, summed across machines.

### Example
```
[.##.] (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}
[...#.] (0,2,3,4) (2,3) (0,4) (0,1,2) (1,2,3,4) {7,5,12,7,2}
[.###.#] (0,1,2,3,4) (0,3,4) (0,1,2,4,5) (1,2) {10,11,11,5,10,5}
```
Expected: 33 (10+12+11)
