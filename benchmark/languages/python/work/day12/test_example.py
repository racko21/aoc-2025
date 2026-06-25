import sys
from collections import Counter

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
        
        if stripped.endswith(':') and stripped[:-1].isdigit():
            if current_shape_idx is not None and current_shape_lines:
                shapes.append(parse_shape(current_shape_lines))
            current_shape_idx = int(stripped[:-1])
            current_shape_lines = []
            i += 1
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

def get_all_placements(shape_cells, width, height):
    orientations = get_orientations(shape_cells)
    placements = set()
    
    for orient in orientations:
        cells = list(orient)
        max_r = max(r for r, c in cells)
        max_c = max(c for r, c in cells)
        
        for dr in range(height - max_r):
            for dc in range(width - max_c):
                placed = frozenset((r + dr, c + dc) for r, c in cells)
                placements.add(placed)
    
    return list(placements)

def can_fit(width, height, present_list, shape_placements):
    if not present_list:
        return True
    
    total_cells_needed = sum(len(next(iter(shape_placements[idx]))) for idx in present_list)
    if total_cells_needed > width * height:
        return False
    
    # Build cell_to_placements lookup
    cell_to_placements = {}
    for idx in set(present_list):
        for p in shape_placements[idx]:
            for cell in p:
                key = (idx, cell)
                if key not in cell_to_placements:
                    cell_to_placements[key] = []
                cell_to_placements[key].append(p)
    
    remaining_counts = Counter(present_list)
    
    def find_first_free(occupied):
        for r in range(height):
            for c in range(width):
                if (r, c) not in occupied:
                    return (r, c)
        return None
    
    def backtrack(occupied, remaining_counts):
        cell = find_first_free(occupied)
        if cell is None:
            return not any(remaining_counts.values())
        
        # Check if remaining_counts is empty (all placed)
        if not remaining_counts:
            return True
        
        for idx in list(remaining_counts.keys()):
            if remaining_counts[idx] == 0:
                continue
            
            placements_for_cell = cell_to_placements.get((idx, cell), [])
            
            for placement in placements_for_cell:
                if not (placement & occupied):
                    remaining_counts[idx] -= 1
                    if remaining_counts[idx] == 0:
                        del remaining_counts[idx]
                    
                    result = backtrack(occupied | placement, remaining_counts)
                    
                    if idx not in remaining_counts:
                        remaining_counts[idx] = 0
                    remaining_counts[idx] += 1
                    if remaining_counts[idx] == 0:
                        del remaining_counts[idx]
                    
                    if result:
                        return True
        
        return False
    
    return backtrack(frozenset(), remaining_counts)

shapes, regions = parse_input('example.txt')

for region_idx, (width, height, counts) in enumerate(regions):
    present_list = []
    for shape_idx, cnt in enumerate(counts):
        present_list.extend([shape_idx] * cnt)
    
    shape_placements = {}
    for shape_idx in set(present_list):
        pls = get_all_placements(shapes[shape_idx], width, height)
        shape_placements[shape_idx] = pls
        print(f"  Shape {shape_idx} in {width}x{height}: {len(pls)} placements")
    
    result = can_fit(width, height, present_list, shape_placements)
    print(f"Region {region_idx} ({width}x{height}, {present_list}): {result}")

print("Expected: regions 0 and 1 fit (True, True, False)")
