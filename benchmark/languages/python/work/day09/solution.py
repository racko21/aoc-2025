import sys
import bisect

def solve():
    with open('input.txt') as f:
        lines = [l.strip() for l in f if l.strip()]
    
    red_tiles = []
    for line in lines:
        x, y = map(int, line.split(','))
        red_tiles.append((x, y))
    
    n = len(red_tiles)
    
    # -------------------------
    # Part 1: Largest rectangle using any two red tiles as opposite corners
    # Area = (|x1-x2|+1) * (|y1-y2|+1) (inclusive tile count)
    # -------------------------
    best1 = 0
    for i in range(n):
        for j in range(i+1, n):
            x1, y1 = red_tiles[i]
            x2, y2 = red_tiles[j]
            area = (abs(x1-x2)+1) * (abs(y1-y2)+1)
            if area > best1:
                best1 = area
    
    print(f"Part 1: {best1}")
    
    # -------------------------
    # Part 2: Rectangle must use only red or green tiles
    # The polygon is formed by connecting consecutive red tiles (wrapping) with H/V segments
    # Interior is also green
    # -------------------------
    
    # Build polygon segments
    segments = []
    for i in range(n):
        a = red_tiles[i]
        b = red_tiles[(i+1) % n]
        segments.append((a, b))
    
    # Separate into vertical and horizontal segments
    vertical_segs = []   # (x, y_min, y_max)
    horizontal_segs = [] # (y, x_min, x_max)
    
    for (a, b) in segments:
        ax, ay = a
        bx, by = b
        if ax == bx:
            vertical_segs.append((ax, min(ay, by), max(ay, by)))
        elif ay == by:
            horizontal_segs.append((ay, min(ax, bx), max(ax, bx)))
        else:
            print(f"ERROR: diagonal segment {a} -> {b}", file=sys.stderr)
    
    # For a rectilinear polygon, for each row y, we can find the inside x-intervals
    # using the scanline method:
    # - For each vertical segment (x, y_min, y_max), it "crosses" row y if y_min <= y < y_max
    #   (half-open convention to handle vertices correctly)
    # - Sort crossing x-values; inside is between pairs [x0,x1], [x2,x3], ...
    # - Also include horizontal segments at that exact y
    
    # Event y-values (where polygon structure changes)
    event_ys = sorted(set(y for x, y in red_tiles))
    
    def get_vert_crossings(y, use_halfopen=True):
        """Get x-values of vertical segments crossing row y."""
        crossings = []
        for (x, y_min, y_max) in vertical_segs:
            if use_halfopen:
                # Half-open: y_min <= y < y_max
                if y_min <= y < y_max:
                    crossings.append(x)
            else:
                # Fully open: y_min < y < y_max (for non-event interior rows)
                if y_min < y < y_max:
                    crossings.append(x)
        crossings.sort()
        return crossings
    
    def crossings_to_intervals(crossings):
        """Convert sorted list of crossing x-values to inside intervals."""
        intervals = []
        for i in range(0, len(crossings), 2):
            if i+1 < len(crossings):
                intervals.append((crossings[i], crossings[i+1]))
        return intervals
    
    def merge_intervals(ivs):
        """Merge a list of (lo, hi) intervals."""
        if not ivs:
            return []
        ivs = sorted(ivs)
        merged = [list(ivs[0])]
        for lo, hi in ivs[1:]:
            if lo <= merged[-1][1]:
                merged[-1][1] = max(merged[-1][1], hi)
            else:
                merged.append([lo, hi])
        return [(a, b) for a, b in merged]
    
    def get_inside_intervals_event(y):
        """Get inside x-intervals for an event row y (boundary included)."""
        crossings = get_vert_crossings(y, use_halfopen=True)
        vert_intervals = crossings_to_intervals(crossings)
        horiz_at_y = [(x_min, x_max) for (hy, x_min, x_max) in horizontal_segs if hy == y]
        all_intervals = vert_intervals + horiz_at_y
        return merge_intervals(all_intervals)
    
    def get_inside_intervals_interior(y):
        """Get inside x-intervals for an interior (non-event) row y."""
        crossings = get_vert_crossings(y, use_halfopen=False)
        return crossings_to_intervals(crossings)
    
    def x_range_inside(intervals, xa, xb):
        """Check if [xa, xb] is fully inside one of the intervals."""
        for (lo, hi) in intervals:
            if lo <= xa and xb <= hi:
                return True
        return False
    
    # Precompute inside intervals for each event row and each interior slab
    event_row_intervals = {}
    for y in event_ys:
        event_row_intervals[y] = get_inside_intervals_event(y)
    
    # For slabs between consecutive event rows, precompute interior intervals
    # (interior of slab = rows y with event_ys[i] < y < event_ys[i+1])
    # All interior rows in a slab have the same intervals (no event changes)
    slab_interior_intervals = {}  # key = (y_low, y_high)
    for i in range(len(event_ys) - 1):
        y_low = event_ys[i]
        y_high = event_ys[i+1]
        if y_high - y_low >= 2:
            # There are interior rows (y_low+1 ... y_high-1)
            mid = y_low + 1  # Any interior row works
            slab_interior_intervals[(y_low, y_high)] = get_inside_intervals_interior(mid)
    
    def rectangle_valid(xa, xb, ya, yb):
        """
        Check if every tile in rectangle [xa,xb] x [ya,yb] is red or green.
        Uses precomputed scanline intervals.
        """
        # Find event rows in [ya, yb]
        i_start = bisect.bisect_left(event_ys, ya)
        i_end = bisect.bisect_right(event_ys, yb)
        
        # Check all event rows in [ya, yb]
        for i in range(i_start, i_end):
            ey = event_ys[i]
            if not x_range_inside(event_row_intervals[ey], xa, xb):
                return False
        
        # Check interior rows in slabs that overlap [ya, yb]
        # Relevant slabs: between event_ys[i] and event_ys[i+1] for i in range
        # We need slabs where there are interior rows in [ya, yb]
        
        # Slabs to check: between event_ys[k] and event_ys[k+1] for k in some range
        # k ranges from i_start-1 (if ya is in interior of that slab) to i_end-1
        
        k_start = max(0, i_start - 1)
        k_end = min(len(event_ys) - 1, i_end)
        
        for k in range(k_start, k_end):
            y_low = event_ys[k]
            y_high = event_ys[k+1]
            # Interior rows: y_low+1 ... y_high-1
            # Intersection with [ya, yb]: max(y_low+1, ya) ... min(y_high-1, yb)
            row_lo = max(y_low + 1, ya)
            row_hi = min(y_high - 1, yb)
            if row_lo <= row_hi:
                # There are interior rows to check
                key = (y_low, y_high)
                if key not in slab_interior_intervals:
                    # No interior rows (consecutive event rows)
                    continue
                intervals = slab_interior_intervals[key]
                if not x_range_inside(intervals, xa, xb):
                    return False
        
        return True
    
    best2 = 0
    for i in range(n):
        for j in range(i+1, n):
            x1, y1 = red_tiles[i]
            x2, y2 = red_tiles[j]
            xa, xb = min(x1,x2), max(x1,x2)
            ya, yb = min(y1,y2), max(y1,y2)
            # Check if rectangle is entirely red/green
            if rectangle_valid(xa, xb, ya, yb):
                area = (xb-xa+1) * (yb-ya+1)
                if area > best2:
                    best2 = area
    
    print(f"Part 2: {best2}")

solve()
