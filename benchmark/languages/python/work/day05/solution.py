def merge_ranges(ranges):
    """Merge overlapping/adjacent ranges and return sorted list of [lo, hi]."""
    if not ranges:
        return []
    sorted_ranges = sorted(ranges, key=lambda r: r[0])
    merged = [sorted_ranges[0][:]]
    for lo, hi in sorted_ranges[1:]:
        if lo <= merged[-1][1] + 1:
            # Overlapping or adjacent — extend
            merged[-1][1] = max(merged[-1][1], hi)
        else:
            merged.append([lo, hi])
    return merged


def solve():
    with open("input.txt") as f:
        content = f.read()

    parts = content.strip().split("\n\n")
    range_lines = parts[0].strip().split("\n")
    id_lines = parts[1].strip().split("\n") if len(parts) > 1 else []

    # Parse ranges
    ranges = []
    for line in range_lines:
        lo, hi = line.strip().split("-")
        ranges.append([int(lo), int(hi)])

    # Parse available IDs
    available_ids = [int(x.strip()) for x in id_lines if x.strip()]

    # --- Part 1 ---
    fresh_count = 0
    for ingredient_id in available_ids:
        for lo, hi in ranges:
            if lo <= ingredient_id <= hi:
                fresh_count += 1
                break

    # --- Part 2 ---
    merged = merge_ranges(ranges)
    total_fresh = sum(hi - lo + 1 for lo, hi in merged)

    print(f"Part 1: {fresh_count}")
    print(f"Part 2: {total_fresh}")


solve()
