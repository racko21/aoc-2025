def count_neighbors(grid, r, c):
    """Count the number of '@' in the 8 adjacent cells."""
    rows = len(grid)
    cols = len(grid[0])
    count = 0
    for dr in [-1, 0, 1]:
        for dc in [-1, 0, 1]:
            if dr == 0 and dc == 0:
                continue
            nr, nc = r + dr, c + dc
            if 0 <= nr < rows and 0 <= nc < cols:
                if grid[nr][nc] == '@':
                    count += 1
    return count

def accessible_rolls(grid):
    """Return set of (r, c) positions of rolls accessible by forklift (< 4 neighbors)."""
    accessible = set()
    for r in range(len(grid)):
        for c in range(len(grid[0])):
            if grid[r][c] == '@':
                if count_neighbors(grid, r, c) < 4:
                    accessible.add((r, c))
    return accessible

def solve(filename):
    with open(filename) as f:
        lines = [line.rstrip('\n') for line in f.readlines()]
    
    # Parse into a mutable grid (list of lists)
    grid = [list(line) for line in lines]
    
    # Part 1: count accessible rolls in initial state
    part1 = len(accessible_rolls(grid))
    
    # Part 2: iteratively remove accessible rolls until none remain
    total_removed = 0
    while True:
        to_remove = accessible_rolls(grid)
        if not to_remove:
            break
        total_removed += len(to_remove)
        for (r, c) in to_remove:
            grid[r][c] = '.'
    
    part2 = total_removed
    
    return part1, part2

p1, p2 = solve('input.txt')
print(f"Part 1: {p1}")
print(f"Part 2: {p2}")
