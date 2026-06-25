import sys

def parse_ranges(text):
    """Parse comma-separated ranges like 'a-b,c-d,...'"""
    text = text.strip().replace('\n', '').replace(' ', '')
    ranges = []
    for part in text.split(','):
        part = part.strip()
        if not part:
            continue
        a, b = part.split('-')
        ranges.append((int(a), int(b)))
    return ranges

def is_in_ranges(n, ranges):
    """Check if n falls in any of the ranges."""
    for a, b in ranges:
        if a <= n <= b:
            return True
    return False

def generate_double_repeat_candidates(ranges):
    """
    Generate all numbers of the form ss (s concatenated with itself, no leading zeros).
    s must not have leading zeros (unless s itself is just '0', but '00' has leading zero).
    So s ranges from 1..9, 10..99, 100..999, etc. (no leading zeros).
    The resulting ss = int(s_str + s_str).
    We check if it falls in any range.
    """
    # Find max value in ranges to limit search
    max_val = max(b for a, b in ranges)
    
    candidates = set()
    
    # s_str has length L, ss has length 2L
    # ss <= max_val means 2L <= len(str(max_val)) approximately
    max_len = len(str(max_val))
    
    # L from 1 to max_len//2 (since 2L <= max_len+1 to be safe)
    for L in range(1, max_len // 2 + 2):
        # s ranges from 10^(L-1) to 10^L - 1 (no leading zeros)
        # For L=1, s from 1..9 (s='0' would give '00' with leading zero, skip)
        s_start = 10 ** (L - 1) if L > 1 else 1
        s_end = 10 ** L - 1
        
        for s in range(s_start, s_end + 1):
            s_str = str(s)
            ss_str = s_str + s_str
            ss = int(ss_str)
            if ss > max_val:
                break  # s is increasing, so ss is too
            candidates.add(ss)
    
    return candidates

def generate_multi_repeat_candidates(ranges):
    """
    Generate all numbers of the form s repeated k times (k >= 2), no leading zeros.
    """
    max_val = max(b for a, b in ranges)
    max_digits = len(str(max_val))
    
    candidates = set()
    
    # s has length L, repeated k times => total length L*k
    # L*k <= max_digits (approximately, could be max_digits+1 but we check)
    for L in range(1, max_digits + 1):
        # For k repetitions, total length = L*k
        # We need L*k <= max_digits (roughly)
        # k >= 2
        for k in range(2, max_digits // L + 1):
            # s ranges: no leading zeros
            s_start = 10 ** (L - 1) if L > 1 else 1
            s_end = 10 ** L - 1
            
            for s in range(s_start, s_end + 1):
                s_str = str(s)
                repeated_str = s_str * k
                val = int(repeated_str)
                if val > max_val:
                    break
                candidates.add(val)
    
    return candidates

def solve(input_file):
    with open(input_file) as f:
        text = f.read()
    
    ranges = parse_ranges(text)
    
    # Part 1: numbers of form ss (repeated exactly twice)
    candidates1 = generate_double_repeat_candidates(ranges)
    part1 = sum(n for n in candidates1 if is_in_ranges(n, ranges))
    
    # Part 2: numbers of form s repeated k>=2 times
    candidates2 = generate_multi_repeat_candidates(ranges)
    part2 = sum(n for n in candidates2 if is_in_ranges(n, ranges))
    
    print(f"Part 1: {part1}")
    print(f"Part 2: {part2}")

solve('input.txt')
