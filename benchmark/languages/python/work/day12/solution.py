import sys
from collections import Counter, defaultdict

def parse_input(filename):
    with open(filename) as f:
        content = f.read()
    
    all_lines = content.strip().split('\n')
    
    shapes = []
    regions = []
    
    current_shape_idx = None
    current_shape_lines = []
    
    i = 0
    while i < len(all_lines):
        line = all_lines[i]
        stripped = line.strip()
        
        # Shape header: digits followed by colon
        if stripped.endswith(':') and stripped[:-1].isdigit():
            if current_shape_idx is not None and current_shape_lines:
                shapes.append(parse_shape(current_shape_lines))
            current_shape_idx = int(stripped[:-1])
            current_shape_lines = []
            i += 1
        # Region line: "WxH: ..."
        elif 'x' in stripped and ':' in stripped and not set(stripped.split(':')[0]) <= {'#', '.', ' '}:
            if current_shape_idx is not None and current_shape_lines:
                shapes.append(parse_shape(current_shape_lines))
                current_shape_idx = None
                current_shape_lines = []
            colon_idx = stripped.index(':')
            dims = stripped[:colon_idx].strip()
            counts_str = stripped[colon_idx+1:].strip()
            w, h = dims.split('x')
            width, height = int(w), int(h)
            counts = list(map(int, counts_str.split()))
            regions.append((width, height, counts))
            i += 1
        elif current_shape_idx is not None and stripped and all(c in '#.' for c in stripped):
            current_shape_lines.append(stripped)
            i += 1
        else:
            i += 1
    
    if current_shape_idx is not None and current_shape_lines:
        shapes.append(parse_shape(current_shape_lines))
    
    return shapes, regions

def parse_shape(lines):
    cells = set()
    for r, line in enumerate(lines):
        for c, ch in enumerate(line):
            if ch == '#':
                cells.add((r, c))
    return frozenset(cells)

def normalize_shape(cells):
    if not cells:
        return frozenset()
    min_r = min(r for r, c in cells)
    min_c = min(c for r, c in cells)
    return frozenset((r - min_r, c - min_c) for r, c in cells)

def get_orientations(shape):
    cells = list(shape)
    orientations = set()
    current = cells
    for _ in range(4):
        norm = normalize_shape(current)
        orientations.add(norm)
        flipped = normalize_shape([(r, -c) for r, c in norm])
        orientations.add(flipped)
        current = [(c, -r) for r, c in current]
    return list(orientations)

def get_all_placements_bitmask(shape_cells, width, height):
    """Get all valid placements as bitmasks"""
    orientations = get_orientations(shape_cells)
    placements = set()
    
    for orient in orientations:
        cells = list(orient)
        max_r = max(r for r, c in cells)
        max_c = max(c for r, c in cells)
        
        for dr in range(height - max_r):
            for dc in range(width - max_c):
                mask = 0
                for r, c in cells:
                    mask |= (1 << ((r + dr) * width + (c + dc)))
                placements.add(mask)
    
    return list(placements)

def can_fit(width, height, piece_types, shape_placements):
    """
    Try to fit all pieces in a width x height region.
    piece_types: list of shape indices (with repetition)
    shape_placements: dict idx -> list of bitmasks
    
    Uses backtracking with most-constrained-first heuristic.
    """
    if not piece_types:
        return True
    
    total_cells_needed = sum(bin(shape_placements[idx][0]).count('1') for idx in piece_types)
    if total_cells_needed > width * height:
        return False
    
    # Check if any piece type has 0 valid placements at all
    for idx in set(piece_types):
        if not shape_placements[idx]:
            return False
    
    n = len(piece_types)
    
    # Backtracking with bitmask
    # State: occupied (bitmask), which pieces remain
    # Strategy: pick the piece with fewest valid placements given current occupied
    
    def backtrack(occupied, remaining):
        """remaining: list of shape indices still to place"""
        if not remaining:
            return True
        
        # Find piece with minimum valid placements (most constrained)
        best_pos = -1
        best_valid = None
        best_count = float('inf')
        
        seen_types = set()
        for pos, idx in enumerate(remaining):
            if idx in seen_types:
                continue
            seen_types.add(idx)
            valid = [p for p in shape_placements[idx] if not (p & occupied)]
            cnt = len(valid)
            if cnt < best_count:
                best_count = cnt
                best_valid = valid
                best_pos = pos
                if cnt == 0:
                    return False
        
        idx_to_place = remaining[best_pos]
        new_remaining = remaining[:best_pos] + remaining[best_pos+1:]
        
        for placement in best_valid:
            if backtrack(occupied | placement, new_remaining):
                return True
        
        return False
    
    return backtrack(0, list(piece_types))

def solve(filename):
    shapes, regions = parse_input(filename)
    
    sys.stderr.write(f"Parsed {len(shapes)} shapes, {len(regions)} regions\n")
    
    count = 0
    
    for region_idx, (width, height, counts) in enumerate(regions):
        piece_types = []
        for shape_idx, cnt in enumerate(counts):
            piece_types.extend([shape_idx] * cnt)
        
        if not piece_types:
            count += 1
            continue
        
        shape_placements = {}
        for shape_idx in set(piece_types):
            shape_placements[shape_idx] = get_all_placements_bitmask(shapes[shape_idx], width, height)
        
        result = can_fit(width, height, piece_types, shape_placements)
        if result:
            count += 1
        
        if region_idx % 10 == 0:
            sys.stderr.write(f"  Region {region_idx}/{len(regions)}: {width}x{height}, {len(piece_types)} pieces -> {result}\n")
    
    return count

result = solve('input.txt')
print(f"Part 1: {result}")
