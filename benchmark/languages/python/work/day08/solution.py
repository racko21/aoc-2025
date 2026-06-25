def parse(filename):
    points = []
    with open(filename) as f:
        for line in f:
            line = line.strip()
            if line:
                x, y, z = map(int, line.split(','))
                points.append((x, y, z))
    return points

def dist_sq(a, b):
    return (a[0]-b[0])**2 + (a[1]-b[1])**2 + (a[2]-b[2])**2

def make_uf(n):
    parent = list(range(n))
    rank = [0] * n
    size = [1] * n
    return parent, rank, size

def find(parent, x):
    while parent[x] != x:
        parent[x] = parent[parent[x]]
        x = parent[x]
    return x

def union(parent, rank, size, a, b):
    ra, rb = find(parent, a), find(parent, b)
    if ra == rb:
        return False
    if rank[ra] < rank[rb]:
        ra, rb = rb, ra
    parent[rb] = ra
    size[ra] += size[rb]
    if rank[ra] == rank[rb]:
        rank[ra] += 1
    return True

def get_top3_product(parent, size, n):
    unique_roots = set(find(parent, k) for k in range(n))
    circuit_sizes = sorted([size[r] for r in unique_roots], reverse=True)
    while len(circuit_sizes) < 3:
        circuit_sizes.append(1)
    return circuit_sizes[0] * circuit_sizes[1] * circuit_sizes[2]

def solve(filename, part1_limit=1000):
    points = parse(filename)
    n = len(points)
    
    # Generate all pairs with squared distances, sort ascending
    pairs = []
    for i in range(n):
        for j in range(i+1, n):
            d2 = dist_sq(points[i], points[j])
            pairs.append((d2, i, j))
    
    pairs.sort()
    
    parent, rank, size = make_uf(n)
    
    part1_answer = None
    part2_answer = None
    
    for idx, (d2, i, j) in enumerate(pairs):
        # Part 1: capture state after processing part1_limit pairs
        # At idx == part1_limit, we've processed indices 0..part1_limit-1
        if idx == part1_limit and part1_answer is None:
            part1_answer = get_top3_product(parent, size, n)
        
        # Try to merge the two circuits
        merged = union(parent, rank, size, i, j)
        
        # Part 2: check if all nodes are now in one circuit
        if part2_answer is None and merged:
            root = find(parent, 0)
            if size[root] == n:
                part2_answer = points[i][0] * points[j][0]
    
    # If fewer than part1_limit pairs exist, use final state
    if part1_answer is None:
        part1_answer = get_top3_product(parent, size, n)
    
    return part1_answer, part2_answer

p1, p2 = solve('input.txt', part1_limit=1000)
print(f"Part 1: {p1}")
print(f"Part 2: {p2}")
