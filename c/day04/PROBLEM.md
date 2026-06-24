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

## Part 2

Once a roll of paper can be accessed by a forklift, it can be removed. Once
a roll of paper is removed, the forklifts might be able to access more
rolls of paper, which they might also be able to remove. Repeat this
process (removing all currently-accessible rolls in a wave, then
re-checking) until no more rolls of paper are accessible.

How many total rolls of paper could the Elves remove?

### Example
Starting from the same grid as Part 1, removing rolls wave by wave (13,
then 12, then 7, 5, 2, 1, 1, 1, 1) removes a total of **43** rolls before
no more are accessible.
