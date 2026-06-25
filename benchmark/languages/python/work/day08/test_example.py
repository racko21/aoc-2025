import math

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

points = parse('example.txt')
n = len(points)

pairs = []
for i in range(n):
    for j in range(i+1, n):
        d2 = dist_sq(points[i], points[j])
        pairs.append((d2, i, j))
pairs.sort()

parent, rank, size = make_uf(n)

for idx in range(15):
    d2, i, j = pairs[idx]
    d = math.sqrt(d2)
    merged = union(parent, rank, size, i, j)
    print(f"Pair {idx+1}: {points[i]} <-> {points[j]}, dist={d:.2f}, merged={merged}")
    
    # Show circuit state
    from collections import Counter
    roots = [find(parent, k) for k in range(n)]
    sizes = Counter(size[find(parent, k)] for k in range(n))
    # unique circuits
    unique_roots = set(roots)
    circuit_sizes = sorted([size[r] for r in unique_roots], reverse=True)
    print(f"  Circuits: {circuit_sizes}")
    print()
