def parse_worksheet_part1(lines):
    """
    Parse worksheet for Part 1.
    Problems are separated by all-space columns.
    Each problem's number rows are read as full numbers (strip whitespace).
    Operator is on the last row.
    """
    if not lines:
        return []
    
    max_len = max(len(line) for line in lines)
    padded = [line.ljust(max_len) for line in lines]
    num_rows = len(padded)

    def is_separator_col(c):
        return all(padded[r][c] == ' ' for r in range(num_rows))

    # Group consecutive non-separator columns into problems
    problems_cols = []
    current_group = []
    for c in range(max_len):
        if is_separator_col(c):
            if current_group:
                problems_cols.append(current_group)
                current_group = []
        else:
            current_group.append(c)
    if current_group:
        problems_cols.append(current_group)

    problems = []
    for group in problems_cols:
        # Find operator in last row
        op = None
        for c in group:
            ch = padded[num_rows - 1][c]
            if ch in ('+', '*'):
                op = ch
                break
        if op is None:
            continue

        # Extract numbers from all rows except last
        numbers = []
        for r in range(num_rows - 1):
            row_str = ''.join(padded[r][c] for c in group).strip()
            if row_str:
                numbers.append(int(row_str))

        if numbers:
            problems.append((numbers, op))

    return problems


def parse_worksheet_part2(lines):
    """
    Parse worksheet for Part 2.
    Same problem grouping as Part 1.
    Within each problem, each character column is a number:
    - digits go top-to-bottom (most significant at top)
    - problems are read right-to-left
    """
    if not lines:
        return []

    max_len = max(len(line) for line in lines)
    padded = [line.ljust(max_len) for line in lines]
    num_rows = len(padded)

    def is_separator_col(c):
        return all(padded[r][c] == ' ' for r in range(num_rows))

    # Group consecutive non-separator columns into problems
    problems_cols = []
    current_group = []
    for c in range(max_len):
        if is_separator_col(c):
            if current_group:
                problems_cols.append(current_group)
                current_group = []
        else:
            current_group.append(c)
    if current_group:
        problems_cols.append(current_group)

    problems = []
    for group in problems_cols:
        # Find operator in last row
        op = None
        for c in group:
            ch = padded[num_rows - 1][c]
            if ch in ('+', '*'):
                op = ch
                break
        if op is None:
            continue

        # Each column (right-to-left) is a number; digits from rows 0..n-2, top to bottom
        numbers = []
        for c in reversed(group):
            digit_str = ''.join(padded[r][c] for r in range(num_rows - 1)).strip()
            if digit_str:
                numbers.append(int(digit_str))

        if numbers:
            problems.append((numbers, op))

    return problems


def solve(problems):
    total = 0
    for numbers, op in problems:
        if op == '+':
            result = sum(numbers)
        else:  # '*'
            result = 1
            for n in numbers:
                result *= n
        total += result
    return total


def main():
    with open('input.txt', 'r') as f:
        content = f.read()

    lines = content.rstrip('\n').split('\n')

    problems1 = parse_worksheet_part1(lines)
    part1 = solve(problems1)

    problems2 = parse_worksheet_part2(lines)
    part2 = solve(problems2)

    print(f"Part 1: {part1}")
    print(f"Part 2: {part2}")


if __name__ == '__main__':
    main()
