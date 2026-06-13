# Day 4: Printing Department

The rolls of paper (@) are arranged on a large grid.

A roll of paper can be accessed by a forklift if it has **fewer than 4** rolls of paper in its 8 adjacent positions.

## Part 1
Count how many rolls of paper (@) can be accessed by a forklift (have < 4 '@' neighbors).

### Example Input
```
..@@.@@@@.
@@@.@.@.@@
@@@@@.@.@@
@.@@@@..@.
@@.@@@@.@@
.@@@@@@@.@
.@.@.@.@@@
@.@@@.@@@@
.@@@@@@@@.
@.@.@@@.@.
```

### Example Output
13 accessible rolls (marked with x):
```
..xx.xx@x.
x@@.@.@.@@
@@@@@.x.@@
@.@@@@..@.
x@.@@@@.@x
.@@@@@@@.@
.@.@.@.@@@
x.@@@.@@@@
.@@@@@@@@.
x.x.@@@.x.
```
