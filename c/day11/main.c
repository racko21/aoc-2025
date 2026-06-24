#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../utils/utils.h"

/* The reactor's devices form a directed acyclic graph: each device lists
 * the devices its output feeds into. We need the number of distinct
 * paths from "you" to "out", following edges only forward. */

#define MAX_NODES 1024
#define MAX_NAME_LEN 16

static char names[MAX_NODES][MAX_NAME_LEN];
static int num_nodes = 0;

static int *adj[MAX_NODES];
static int adj_len[MAX_NODES];
static int adj_cap[MAX_NODES];

/* Returns the id for a node name, creating a new id if it hasn't been
 * seen before. */
static int intern(const char *name) {
    for (int i = 0; i < num_nodes; i++) {
        if (strcmp(names[i], name) == 0) {
            return i;
        }
    }
    if (num_nodes >= MAX_NODES) {
        fprintf(stderr, "error: too many nodes\n");
        exit(1);
    }
    int id = num_nodes++;
    strncpy(names[id], name, MAX_NAME_LEN - 1);
    names[id][MAX_NAME_LEN - 1] = '\0';
    return id;
}

/* Adds an edge from src to dst, growing src's adjacency list as needed. */
static void add_edge(int src, int dst) {
    if (adj_len[src] >= adj_cap[src]) {
        adj_cap[src] = adj_cap[src] == 0 ? 4 : adj_cap[src] * 2;
        adj[src] = realloc(adj[src], (size_t)adj_cap[src] * sizeof(int));
        if (!adj[src]) {
            fprintf(stderr, "error: out of memory\n");
            exit(1);
        }
    }
    adj[src][adj_len[src]++] = dst;
}

/* Resets all global graph state so a fresh file can be parsed. */
static void reset_graph(void) {
    for (int i = 0; i < num_nodes; i++) {
        free(adj[i]);
        adj[i] = NULL;
        adj_len[i] = 0;
        adj_cap[i] = 0;
    }
    num_nodes = 0;
}

/* Parses the device list file into the global adjacency structures. */
static void parse_graph(const char *path) {
    char *data = read_file(path);
    char *line_state = NULL;
    char *line = strtok_r(data, "\n", &line_state);
    while (line != NULL) {
        char name[MAX_NAME_LEN];
        const char *colon = strchr(line, ':');
        if (!colon) {
            line = strtok_r(NULL, "\n", &line_state);
            continue;
        }
        size_t name_len = (size_t)(colon - line);
        if (name_len >= MAX_NAME_LEN) {
            name_len = MAX_NAME_LEN - 1;
        }
        memcpy(name, line, name_len);
        name[name_len] = '\0';
        int src = intern(name);

        char *rest = (char *)colon + 1;
        char *tok_state = NULL;
        char *tok = strtok_r(rest, " ", &tok_state);
        while (tok != NULL) {
            int dst = intern(tok);
            add_edge(src, dst);
            tok = strtok_r(NULL, " ", &tok_state);
        }
        line = strtok_r(NULL, "\n", &line_state);
    }
    free(data);
}

static long long *memo;

/* Returns the number of distinct paths from node `id` to the "out" node,
 * memoizing results since the graph is a DAG. */
static long long count_paths(int id, int out_id) {
    if (id == out_id) {
        return 1;
    }
    if (memo[id] >= 0) {
        return memo[id];
    }
    long long total = 0;
    for (int i = 0; i < adj_len[id]; i++) {
        total += count_paths(adj[id][i], out_id);
    }
    memo[id] = total;
    return total;
}

long long solve_part1(const char *path) {
    reset_graph();
    parse_graph(path);
    int you_id = intern("you");
    int out_id = intern("out");

    memo = malloc((size_t)num_nodes * sizeof(long long));
    for (int i = 0; i < num_nodes; i++) {
        memo[i] = -1;
    }
    long long result = count_paths(you_id, out_id);
    free(memo);
    return result;
}

static long long *memo2; /* indexed as node * 4 + has_dac * 2 + has_fft */
static int dac_id, fft_id, out_id2;

/* Returns the number of paths from `id` to "out" that, including any
 * nodes already visited (has_dac/has_fft), end up having visited both
 * dac and fft by the time they reach "out". */
static long long count_paths2(int id, int has_dac, int has_fft) {
    if (id == dac_id) {
        has_dac = 1;
    }
    if (id == fft_id) {
        has_fft = 1;
    }
    if (id == out_id2) {
        return (has_dac && has_fft) ? 1 : 0;
    }
    int key = id * 4 + has_dac * 2 + has_fft;
    if (memo2[key] >= 0) {
        return memo2[key];
    }
    long long total = 0;
    for (int i = 0; i < adj_len[id]; i++) {
        total += count_paths2(adj[id][i], has_dac, has_fft);
    }
    memo2[key] = total;
    return total;
}

long long solve_part2(const char *path) {
    reset_graph();
    parse_graph(path);
    int svr_id = intern("svr");
    dac_id = intern("dac");
    fft_id = intern("fft");
    out_id2 = intern("out");

    memo2 = malloc((size_t)num_nodes * 4 * sizeof(long long));
    for (int i = 0; i < num_nodes * 4; i++) {
        memo2[i] = -1;
    }
    long long result = count_paths2(svr_id, 0, 0);
    free(memo2);
    return result;
}

#ifndef UNIT_TEST
static double elapsed_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000.0 + (b.tv_nsec - a.tv_nsec) / 1e6;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc > 1) {
        part = atoi(argv[1]);
    }

    struct timespec t0, t1;
    if (part == 0 || part == 1) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans1 = solve_part1("day11/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("day11/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }
    return 0;
}
#endif
