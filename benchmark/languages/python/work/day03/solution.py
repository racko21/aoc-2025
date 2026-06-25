def max_joltage(bank, k):
    """Select k digits from bank (maintaining order) to form the largest k-digit number."""
    n = len(bank)
    if k > n:
        return 0
    
    result = []
    start = 0
    for i in range(k):
        # We need to pick digit i out of k
        # We can pick from index start to index (n - k + i) inclusive
        # because we need to leave (k - i - 1) digits after our pick
        end = n - (k - i - 1)  # exclusive upper bound for search
        
        # Find the maximum digit in bank[start:end]
        best_val = -1
        best_pos = start
        for j in range(start, end):
            if int(bank[j]) > best_val:
                best_val = int(bank[j])
                best_pos = j
        
        result.append(bank[best_pos])
        start = best_pos + 1
    
    return int(''.join(result))

def solve():
    with open('input.txt', 'r') as f:
        lines = [line.strip() for line in f if line.strip()]
    
    # Part 1: select 2 digits
    total1 = sum(max_joltage(line, 2) for line in lines)
    
    # Part 2: select 12 digits
    total2 = sum(max_joltage(line, 12) for line in lines)
    
    print(f"Part 1: {total1}")
    print(f"Part 2: {total2}")

solve()
