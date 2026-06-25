def solve(filename):
    with open(filename) as f:
        rotations = [line.strip() for line in f if line.strip()]

    pos = 50
    part1 = 0
    part2 = 0

    for rot in rotations:
        direction = rot[0]
        dist = int(rot[1:])

        if direction == 'L':
            # Moving left (decreasing), new_pos = (pos - dist) % 100
            # We hit 0 at step pos, pos+100, pos+200, ... (if pos > 0)
            # If pos == 0, we hit 0 at step 100, 200, ...
            if pos == 0:
                # First hit at step 100
                zeros_passed = dist // 100
            else:
                # First hit at step pos (when we decrement from pos to 0)
                if dist >= pos:
                    zeros_passed = 1 + (dist - pos) // 100
                else:
                    zeros_passed = 0
            new_pos = (pos - dist) % 100

        else:  # R
            # Moving right (increasing), new_pos = (pos + dist) % 100
            # We hit 0 at step (100 - pos), (200 - pos), ...
            # If pos == 0, we hit 0 at step 100, 200, ...
            if pos == 0:
                # First hit at step 100
                zeros_passed = dist // 100
            else:
                # First hit at step (100 - pos)
                steps_to_first_zero = 100 - pos
                if dist >= steps_to_first_zero:
                    zeros_passed = 1 + (dist - steps_to_first_zero) // 100
                else:
                    zeros_passed = 0
            new_pos = (pos + dist) % 100

        # Part 1: count if final position is 0
        if new_pos == 0:
            part1 += 1

        # Part 2: count all times dial points at 0 (including mid-rotation)
        # zeros_passed counts all times 0 is hit including final if final==0
        part2 += zeros_passed

        pos = new_pos

    return part1, part2


p1, p2 = solve("input.txt")
print(f"Part 1: {p1}")
print(f"Part 2: {p2}")
