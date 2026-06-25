import sys
from collections import deque

def solve(grid):
    rows = len(grid)
    cols = len(grid[0]) if rows > 0 else 0
    
    # Find start position S
    start_col = -1
    for c in range(cols):
        if grid[0][c] == 'S':
            start_col = c
            break
    
    # Part 1: simulate beams
    # A beam is represented as a column position moving downward
    # When it hits a ^, it splits into beams going left and right
    # We need to count how many ^ splitters are hit
    
    # BFS/DFS simulation for Part 1
    # State: (row, col) for a downward-moving beam
    # But beams can also be lateral (after splitting), and those beams
    # move down too when they encounter a splitter
    # Wait - re-reading the problem:
    # "a new tachyon beam continues from the immediate left and from the immediate right of the splitter"
    # These new beams also move DOWNWARD
    # So beams always move downward, they just start at different columns
    
    # Actually let me re-read more carefully...
    # "Tachyon beams always move downward"
    # When a beam hits ^ at (r, c):
    #   - beam from left: starts at (r, c-1) moving downward  
    #   - beam from right: starts at (r, c+1) moving downward
    # Wait but looking at the example output, the beams shown as | go
    # from S downward, and when split, the new beams also go downward
    # from adjacent cells.
    
    # Let me trace the example:
    # S is at row 0, col 7
    # Beam goes down col 7: row 1 (.), row 2 (^) - HIT
    # Split: new beams at (2, 6) and (2, 8), both going downward
    # Beam at col 6 from row 2: row 3 (.), row 4 (^) - HIT  
    # Beam at col 8 from row 2: row 3 (.), row 4 (^) - HIT
    # etc.
    
    # So Part 1 = number of unique ^ cells that are reached by any beam
    
    # For Part 1: simulate with BFS
    # Each "beam" is defined by (start_row, col) going downward
    # It travels down until hitting a ^ or exiting
    # When hitting ^ at row r, col c: spawn beams at (r, c-1) and (r, c+1)
    
    # But multiple beams can hit the same ^, and we should count each ^ once
    # Actually wait - "a tachyon beam is split a total of 21 times"
    # In the example there are 21 ^ symbols. So Part 1 = number of ^ that are reached.
    
    # Let me verify: count ^ in example
    # Row 2: 1 ^, Row 4: 2 ^, Row 6: 3 ^, Row 8: 3 ^, Row 10: 4 ^, Row 12: 3 ^, Row 14: 5 ^
    # Total: 1+2+3+3+4+3+5 = 21. Yes!
    
    # So Part 1 = count of ^ that are hit by at least one beam
    
    # Simulation: 
    # Use a set of "active beam columns at each row" 
    # Actually, let's think of it differently:
    # A beam starting at (start_row, col) going down will hit the first ^ below it
    # in column col at some row r. That ^ is "hit". Then two new beams start at (r, col-1) and (r, col+1).
    
    # We can do BFS where each item is (row, col) = start position of a downward beam
    # and we mark visited to avoid reprocessing
    
    def simulate_part1():
        # Each beam is (row, col) - position from which it starts going down
        # Actually beams start AT a cell and move down
        # Let's say a beam state is (current_row, col) where it's currently at that position going down
        
        # Simpler: a beam token is (row, col) meaning "beam entering cell (row, col) from above"
        # If (row, col) is out of bounds: beam exits, done
        # If grid[row][col] == '.': beam continues to (row+1, col)
        # If grid[row][col] == 'S': beam continues to (row+1, col) (S is entry)
        # If grid[row][col] == '^': beam is split; count this ^ as hit
        #   spawn beams entering (row, col-1) from the right? No...
        #   "new tachyon beam continues from the immediate left and from the immediate right of the splitter"
        #   So beams start at (row, col-1) and (row, col+1) going DOWNWARD
        #   That means they enter (row+1, col-1) and (row+1, col+1)? 
        #   Wait - "continues from immediate left/right" - they are at row level of splitter, 
        #   then they go downward from there
        
        # Looking at example diagram after first split at row 2, col 7:
        # The beams shown at row 2 are at col 6 and col 8 (the |^| pattern)
        # Then they continue down from row 3 onwards
        # So when a beam hits ^ at (r, c):
        #   - new beam enters (r, c-1) going downward (so next step is (r+1, c-1))
        #   Wait, but (r, c-1) shows a | in the diagram for row 2
        #   Actually the beam at col 6 starts AT row 2 (shown as |), then continues to row 3, etc.
        
        # So: when beam hits ^ at (r, c):
        #   spawn beam at col c-1 starting from row r (it occupies (r, c-1))
        #   spawn beam at col c+1 starting from row r
        #   Both go downward from row r
        
        # But then (r, c-1) might also be a ^! In that case it would split again.
        # Actually looking at the example at row 4: ......^.^......
        # When beams at col 6 and col 8 hit the ^ at row 4:
        #   col 6 hits ^ at (4,6): spawns at (4,5) and (4,7)
        #   col 8 hits ^ at (4,8): spawns at (4,7) and (4,9)
        # But (4,7) is '.', so those beams just continue down.
        # The diagram shows: .....|^|^|..... at row 4
        # So at row 4 we have beams at cols 5,6,7,8,9? That's 5 beams.
        # Wait: |^|^| means col5=|, col6=^, col7=|, col8=^, col9=|
        # So spawned beams at (4,5), (4,7) from splitting (4,6)
        # And spawned beams at (4,7), (4,9) from splitting (4,8)
        # (4,7) is counted once even though two beams spawn there.
        
        # The beam at (r, c-1) and (r, c+1): these are "at that row" and then go down from row r+1
        # But wait, if (r, c-1) is itself a ^, does it split?
        # Looking at the problem: beams move downward and hit splitters.
        # The spawned beams are AT the left/right of the splitter and then move downward.
        # So the spawned beams are horizontal? No - they move downward from those positions.
        
        # I think the model is:
        # A beam enters a cell from above (going down).
        # If cell is '.' or 'S': passes through, enters cell below.
        # If cell is '^': beam stops here. New beams are spawned at (row, col-1) and (row, col+1),
        #   entering those cells (at the same row level), and they go DOWNWARD from there.
        # But what if (row, col-1) is also '^'? Then that cell gets hit too.
        
        # Actually I think the spawned beams move downward STARTING from the row of the splitter.
        # So they enter (row+1, col-1) and (row+1, col+1)? No that doesn't match the diagram.
        
        # Let me re-read: "a new tachyon beam continues from the immediate left and from the immediate right"
        # "continues" suggests they keep going downward.
        # In the diagram, after split at (2,7), we see | at (2,6) and (2,8).
        # This means the beams ARE at row 2, cols 6 and 8.
        # Then they travel down from row 3 onwards.
        # So a spawned beam occupies the cell at (r, c±1) at row r, then goes to (r+1, c±1), etc.
        
        # So the beam at (r, c-1) moving down: it IS at row r, so if (r, c-1) == '^', it would split.
        # But in the example, (2, 6) is '.' so no issue there.
        
        # Model: beam token = (row, col) meaning beam is currently at (row, col) moving downward.
        # Process:
        #   if out of bounds: done
        #   if '.': continue to (row+1, col)  
        #   if 'S': continue to (row+1, col)
        #   if '^': mark as hit, spawn tokens at (row, col-1) and (row, col+1) IF in bounds
        #           (but don't go to (row+1, col))
        
        # To avoid infinite loops: track visited (row, col) pairs as beam entry points
        # Actually beams only move downward so no cycles possible!
        # A beam at (r, c) will always move to larger r values, so no cycles.
        # But we might visit the same cell from multiple beams (like (4,7) above).
        # We should deduplicate to avoid exponential blowup.
        
        visited = set()  # (row, col) cells where a beam has been
        hit_splitters = set()  # splitter ^ cells that were hit
        
        queue = deque()
        # Start: beam enters at (0, start_col)
        queue.append((0, start_col))
        
        while queue:
            r, c = queue.popleft()
            
            # Skip if out of bounds
            if r < 0 or r >= rows or c < 0 or c >= cols:
                continue
            
            # Skip if already visited this cell
            if (r, c) in visited:
                continue
            visited.add((r, c))
            
            cell = grid[r][c]
            if cell == '.' or cell == 'S':
                # Continue downward
                queue.append((r+1, c))
            elif cell == '^':
                # Split
                hit_splitters.add((r, c))
                # Spawn beams to left and right at same row
                queue.append((r, c-1))
                queue.append((r, c+1))
        
        return len(hit_splitters)
    
    part1 = simulate_part1()
    
    # Part 2: Many-worlds interpretation
    # Each time a particle reaches a splitter, the timeline splits into 2.
    # We need to count total timelines after all paths complete.
    # 
    # Key insight: a "timeline" is a specific sequence of left/right choices at splitters.
    # The number of timelines = number of leaf paths in the tree of choices.
    # 
    # Actually: total timelines = sum over all possible paths from start to exit (or off-grid).
    # Each timeline is a single path through the grid.
    # When a particle reaches a ^ at some point in its timeline, the timeline splits into 2.
    # So if a particle hits N splitters on its path, it contributes 2^N... no wait.
    # 
    # Each split doubles the current number of timelines.
    # Starting with 1 timeline:
    # - hits splitter 1: 2 timelines
    # - in each of those timelines, the particle may hit more splitters
    # 
    # So the total number of timelines = number of distinct leaf nodes in the 
    # binary tree of choices.
    # 
    # Actually it's simpler: total timelines = number of distinct paths from start to termination.
    # Each path is a sequence of left/right decisions. Some paths might be longer, some shorter.
    # 
    # Let's think recursively:
    # f(r, c) = number of timelines that start at cell (r, c) going downward
    # if out of bounds: 1 (this timeline ended)
    # if '.': f(r+1, c)
    # if '^': f(r, c-1) + f(r, c+1)
    # if 'S': f(r+1, c)
    # 
    # But this can have exponential computation due to overlapping subproblems!
    # We need memoization.
    # 
    # Wait - can we have cycles? No, because:
    # - '.' and 'S' only go to row r+1 (increasing row)
    # - '^' spawns at (r, c-1) and (r, c+1) - same row but different columns
    #   Those cells must be '.' or out-of-bounds (since splitters are isolated? not guaranteed)
    #   Actually they COULD be '^' themselves, and then they'd spawn at (r, c±1±1)...
    #   Hmm, could we have a chain of ^ at the same row? That seems unlikely but possible.
    #   In that case: beam at (r, c) hits ^, spawns (r, c-1) which hits ^, spawns (r, c-2)...
    #   This could cycle if c-1 spawns at c which spawns at c-1... NO! 
    #   ^ spawns LEFT and RIGHT. If (r, c) -> spawns (r, c-1) which is ^, 
    #   then (r, c-1) spawns (r, c-2) and (r, c). But (r, c) was already visited!
    #   With memoization/visited tracking this is fine, but the recursion value might be wrong
    #   if we have mutual dependencies.
    # 
    # Let me think about this differently for Part 2.
    # 
    # The example says 40 timelines total.
    # 
    # For Part 2, since each timeline is independent, and we want to count total timelines,
    # we can think of it as counting the number of paths in a DAG.
    # 
    # f(r, c) = number of distinct paths/timelines starting from beam at (r, c) going down
    # 
    # Base case: r >= rows or c < 0 or c >= cols -> return 1 (timeline exits = 1 path)
    # 
    # For '.': f(r, c) = f(r+1, c)
    # For '^': f(r, c) = f(r, c-1) + f(r, c+1)
    # For 'S': f(r, c) = f(r+1, c)
    # 
    # But for '^' spawning same-row cells: can lead to cycles if there are adjacent ^s.
    # Let's check if the input has adjacent ^ at the same row... 
    # Looking at the input: "^.^" patterns seem common, with '.' between them, so no adjacent ^.
    # But we should handle it properly anyway.
    # 
    # If no adjacent ^s at same row (which seems to be the case from the input structure),
    # then the spawned cells (r, c±1) are always '.' or out-of-bounds.
    # In that case, f(r, c±1) = f(r+1, c±1) (since they're '.').
    # 
    # With memoization, this is efficient.
    # 
    # Let's implement with memoization using lru_cache or a dict.
    
    memo = {}
    
    def count_timelines(r, c):
        # Returns number of timelines starting from beam at (r, c) going down
        if r < 0 or r >= rows or c < 0 or c >= cols:
            return 1  # beam exits grid = 1 timeline ends here
        
        if (r, c) in memo:
            return memo[(r, c)]
        
        cell = grid[r][c]
        if cell == '.' or cell == 'S':
            result = count_timelines(r+1, c)
        elif cell == '^':
            # Split: left and right
            result = count_timelines(r, c-1) + count_timelines(r, c+1)
        else:
            result = count_timelines(r+1, c)
        
        memo[(r, c)] = result
        return result
    
    # Check for cycles in the ^ same-row case
    # Actually if there are adjacent ^s, we could have infinite recursion.
    # Let me handle this carefully.
    # Actually with memoization, if we encounter a cycle, we'd get stuck in recursion.
    # Let me check: can f(r,c) depend on itself?
    # f(r, c) where grid[r][c] = '^' -> f(r, c-1) + f(r, c+1)
    # f(r, c-1) where grid[r][c-1] = '^' -> f(r, c-2) + f(r, c)  <- cycle!
    # 
    # So if there are adjacent ^s we have a problem.
    # Let me check the input for adjacent ^s at same row...
    
    # For safety, let me use an iterative approach with cycle detection
    # or check if adjacent ^s exist.
    
    # Check for adjacent ^s:
    has_adjacent_carets = False
    for r in range(rows):
        for c in range(cols-1):
            if grid[r][c] == '^' and grid[r][c+1] == '^':
                has_adjacent_carets = True
                break
    
    if has_adjacent_carets:
        print("WARNING: adjacent carets found!", file=sys.stderr)
    
    # Increase recursion limit for deep grids
    sys.setrecursionlimit(100000)
    
    part2 = count_timelines(0, start_col)
    
    return part1, part2


def main():
    with open('input.txt') as f:
        lines = f.read().splitlines()
    
    # Remove empty trailing lines
    while lines and not lines[-1].strip():
        lines.pop()
    
    grid = lines
    
    part1, part2 = solve(grid)
    print(f"Part 1: {part1}")
    print(f"Part 2: {part2}")


if __name__ == '__main__':
    main()
