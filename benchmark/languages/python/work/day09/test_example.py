import sys
from collections import defaultdict
import bisect

def solve(filename):
    with open(filename) as f:
        lines = [l.strip() for l in f if l.strip()]
    
    red_tiles = []
    for line in lines:
        x, y = map(int, line.split(','))
        red_tiles.append((x, y))
    
    n = len(red_tiles)
    
    # Part 1: Find largest rectangle using any two red tiles as opposite corners
    best1 = 0
    best1_pair = None
    for i in range(n):
        for j in range(i+1, n):
            x1, y1 = red_tiles[i]
            x2, y2 = red_tiles[j]
            if x1 != x2 and y1 != y2:
                area = abs(x1-x2) * abs(y1-y2)
                if area > best1:
                    best1 = area
                    best1_pair = (red_tiles[i], red_tiles[j])
    
    print(f"Part 1: {best1} (between {best1_pair})")
    # Expected: 50
    
    # Part 2
    segments = []
    for i in range(n):
        a = red_tiles[i]
        b = red_tiles[(i+1) % n]
        segments.append((a, b))
    
    vertical_segs = []
    horizontal_segs = []
    
    for (a, b) in segments:
        ax, ay = a
        bx, by = b
        if ax == bx:
            vertical_segs.append((ax, min(ay, by), max(ay, by)))
        elif ay == by:
            horizontal_segs.append((ay, min(ax, bx), max(ax, bx)))
        else:
            print(f"ERROR: diagonal segment {a} -> {b}", file=sys.stderr)
    
    print("Vertical segments:", vertical_segs)
    print("Horizontal segments:", horizontal_segs)
    
    # Print the grid to visualize
    all_x = [x for x,y in red_tiles]
    all_y = [y for x,y in red_tiles]
    max_y = max(all_y)
    max_x = max(all_x)
    
    # Build green+red set
    rg_set = set(red_tiles)
    for (a, b) in segments:
        ax, ay = a
        bx, by = b
        if ax == bx:
            for yy in range(min(ay,by), max(ay,by)+1):
                rg_set.add((ax, yy))
        elif ay == by:
            for xx in range(min(ax,bx), max(ax,bx)+1):
                rg_set.add((xx, ay))
    
    print("Boundary tiles:")
    for y in range(max_y+2):
        row = ""
        for x in range(max_x+3):
            if (x,y) in set(red_tiles):
                row += '#'
            elif (x,y) in rg_set:
                row += 'X'
            else:
                row += '.'
        print(row)
    
    # Now flood fill interior (complement of exterior)
    # Start from a corner that's outside
    grid = set()
    # All positions in bounding box
    # Exterior: flood fill from (0,0)
    from collections import deque
    exterior = set()
    q = deque()
    # Pick a point we know is outside
    start = (-1, -1)
    q.append(start)
    exterior.add(start)
    while q:
        cx, cy = q.popleft()
        for dx, dy in [(0,1),(0,-1),(1,0),(-1,0)]:
            nx, ny = cx+dx, cy+dy
            if (nx, ny) not in exterior and (nx, ny) not in rg_set:
                if -2 <= nx <= max_x+2 and -2 <= ny <= max_y+2:
                    exterior.add((nx, ny))
                    q.append((nx, ny))
    
    # Interior = everything in bbox not exterior and not already in rg_set -> add to rg_set
    for x in range(max_x+2):
        for y in range(max_y+2):
            if (x,y) not in exterior:
                rg_set.add((x,y))
    
    print("\nWith interior:")
    for y in range(max_y+2):
        row = ""
        for x in range(max_x+3):
            if (x,y) in set(red_tiles):
                row += '#'
            elif (x,y) in rg_set:
                row += 'X'
            else:
                row += '.'
        print(row)
    
    # Check rectangles
    best2 = 0
    best2_pair = None
    for i in range(n):
        for j in range(i+1, n):
            x1, y1 = red_tiles[i]
            x2, y2 = red_tiles[j]
            if x1 != x2 and y1 != y2:
                xa, xb = min(x1,x2), max(x1,x2)
                ya, yb = min(y1,y2), max(y1,y2)
                # Check all tiles in rectangle
                valid = True
                for rx in range(xa, xb+1):
                    for ry in range(ya, yb+1):
                        if (rx, ry) not in rg_set:
                            valid = False
                            break
                    if not valid:
                        break
                if valid:
                    area = (xb-xa) * (yb-ya)
                    if area > best2:
                        best2 = area
                        best2_pair = (red_tiles[i], red_tiles[j])
    
    print(f"\nPart 2: {best2} (between {best2_pair})")
    # Expected: 24

solve('example.txt')
