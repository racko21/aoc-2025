import sys
sys.path.insert(0, '.')

def solve(grid):
    from collections import deque
    rows = len(grid)
    cols = len(grid[0]) if rows > 0 else 0
    
    start_col = -1
    for c in range(cols):
        if grid[0][c] == 'S':
            start_col = c
            break
    
    def simulate_part1():
        visited = set()
        hit_splitters = set()
        queue = deque()
        queue.append((0, start_col))
        
        while queue:
            r, c = queue.popleft()
            if r < 0 or r >= rows or c < 0 or c >= cols:
                continue
            if (r, c) in visited:
                continue
            visited.add((r, c))
            
            cell = grid[r][c]
            if cell == '.' or cell == 'S':
                queue.append((r+1, c))
            elif cell == '^':
                hit_splitters.add((r, c))
                queue.append((r, c-1))
                queue.append((r, c+1))
        
        return len(hit_splitters)
    
    part1 = simulate_part1()
    
    memo = {}
    def count_timelines(r, c):
        if r < 0 or r >= rows or c < 0 or c >= cols:
            return 1
        if (r, c) in memo:
            return memo[(r, c)]
        cell = grid[r][c]
        if cell == '.' or cell == 'S':
            result = count_timelines(r+1, c)
        elif cell == '^':
            result = count_timelines(r, c-1) + count_timelines(r, c+1)
        else:
            result = count_timelines(r+1, c)
        memo[(r, c)] = result
        return result
    
    sys.setrecursionlimit(100000)
    part2 = count_timelines(0, start_col)
    return part1, part2

with open('example.txt') as f:
    lines = f.read().splitlines()
while lines and not lines[-1].strip():
    lines.pop()

p1, p2 = solve(lines)
print(f"Part 1: {p1} (expected 21)")
print(f"Part 2: {p2} (expected 40)")
