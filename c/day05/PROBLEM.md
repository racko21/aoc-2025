# Day 5: Cafeteria

Database: list of fresh ingredient ID ranges, blank line, list of available ingredient IDs.

Ranges are inclusive and may overlap. An ID is fresh if it falls in any range.

## Part 1
Count how many available ingredient IDs are fresh.

### Example Input
```
3-5
10-14
16-20
12-18

1
5
8
11
17
32
```

### Example Answer
3 (IDs 5, 11, 17 are fresh)

## Part 2
The available ingredient IDs section is now irrelevant. Count the total
number of distinct ingredient IDs covered by the fresh ranges themselves
(ranges may overlap; merge them before counting, since overlapping IDs
must only be counted once).

### Example
Ranges:
```
3-5
10-14
16-20
12-18
```
Covered IDs: 3,4,5,10,11,12,13,14,15,16,17,18,19,20 → 14 distinct IDs.

### Example Answer
14
