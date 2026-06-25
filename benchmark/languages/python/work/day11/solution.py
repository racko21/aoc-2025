import sys
from functools import lru_cache

def parse_graph(lines):
    graph = {}
    for line in lines:
        line = line.strip()
        if not line:
            continue
        colon = line.index(':')
        src = line[:colon].strip()
        dests_str = line[colon+1:].strip()
        dests = dests_str.split() if dests_str else []
        graph[src] = dests
    return graph

def count_paths_part1(graph, start, end):
    """Count all paths from start to end using memoization."""
    # Build adjacency: node -> list of neighbors
    # We need to handle nodes that might appear as destinations but have no entry
    
    memo = {}
    
    def dfs(node):
        if node == end:
            return 1
        if node in memo:
            return memo[node]
        if node not in graph or not graph[node]:
            memo[node] = 0
            return 0
        total = 0
        for neighbor in graph[node]:
            total += dfs(neighbor)
        memo[node] = total
        return total
    
    return dfs(start)

def count_paths_part2(graph, start, end, required):
    """Count paths from start to end that visit all nodes in required set.
    
    State: (current_node, frozenset of required nodes visited so far)
    """
    required = frozenset(required)
    memo = {}
    
    def dfs(node, visited_required):
        if node == end:
            # Only count if all required nodes visited
            if visited_required == required:
                return 1
            else:
                return 0
        
        state = (node, visited_required)
        if state in memo:
            return memo[state]
        
        if node not in graph or not graph[node]:
            memo[state] = 0
            return 0
        
        total = 0
        for neighbor in graph[node]:
            new_visited = visited_required
            if neighbor in required:
                new_visited = visited_required | frozenset([neighbor])
            total += dfs(neighbor, new_visited)
        
        memo[state] = total
        return total
    
    # Start: check if start itself is in required
    initial_visited = frozenset()
    if start in required:
        initial_visited = frozenset([start])
    
    return dfs(start, initial_visited)

def main():
    with open('input.txt', 'r') as f:
        lines = f.readlines()
    
    graph = parse_graph(lines)
    
    # Part 1: count paths from 'you' to 'out'
    part1 = count_paths_part1(graph, 'you', 'out')
    print(f"Part 1: {part1}")
    
    # Part 2: count paths from 'svr' to 'out' that visit both 'dac' and 'fft'
    part2 = count_paths_part2(graph, 'svr', 'out', ['dac', 'fft'])
    print(f"Part 2: {part2}")

if __name__ == '__main__':
    main()
