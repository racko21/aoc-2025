import sys
from collections import deque

def solve(filename):
    with open(filename) as f:
        lines = [l.strip() for l in f if l.strip()]
    
    red_tiles = []
    for line in lines:
        x, y = map(int, line.split(','))
        red_tiles.append((x, y))
    
    n = len(red_tiles)
    
    # Part 1: Find largest rectangle using any two red tiles as opposite corners
    # Area = (|x1-x2|+1) * (|y1-y2|+1) - INCLUSIVE counting
    best1 = 0
    best1_pair = None
    for i in range(n):
        for j in range(i+1, n):
            x1, y1 = red_tiles[i]
            x2, y2 = red_tiles[j]
            # Allow degenerate rectangles (same row or col gives area = max(1, diff+1))
            area = (abs(x1-x2)+1) * (abs(y1-y2)+1)
            if area > best1:
                best1 = area
                best1_pair = (red_tiles[i], red_tiles[j])
    
    print(f"Part 1: {best1} (between {best1_pair})")
    # Expected: 50
    
    # Build red+green set
    segments = []
    for i in range(n):
        a = red_tiles[i]
        b = red_tiles[(i+1) % n]
        segments.append((a, b))
    
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
    
    all_x = [x for x,y in red_tiles]
    all_y = [y for x,y in red_tiles]
    max_y = max(all_y)
    max_x = max(all_x)
    
    # Flood fill exterior
    exterior = set()
    q = deque()
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
    
    # Interior = everything in bbox not exterior
    for x in range(-1, max_x+2):
        for y in range(-1, max_y+2):
            if (x,y) not in exterior:
                rg_set.add((x,y))
    
    # Check rectangles
    best2 = 0
    best2_pair = None
    for i in range(n):
        for j in range(i+1, n):
            x1, y1 = red_tiles[i]
            x2, y2 = red_tiles[j]
            xa, xb = min(x1,x2), max(x1,x2)
            ya, yb = min(y1,y2), max(y1,y2)
            # Check all tiles in rectangle (inclusive)
            valid = True
            for rx in range(xa, xb+1):
                for ry in range(ya, yb+1):
                    if (rx, ry) not in rg_set:
                        valid = False
                        break
                if not valid:
                    break
            if valid:
                area = (xb-xa+1) * (yb-ya+1)
                if area > best2:
                    best2 = area
                    best2_pair = (red_tiles[i], red_tiles[j])
    
    print(f"Part 2: {best2} (between {best2_pair})")
    # Expected: 24

solve('example.txt')
