#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_NODES 4096
#define MAX_EDGES 32768
#define NAME_LEN  32

/* ---- string interning ---- */
static char names[MAX_NODES][NAME_LEN];
static int  name_count = 0;

static int find_or_add(const char *s) {
    for (int i = 0; i < name_count; i++)
        if (strcmp(names[i], s) == 0) return i;
    if (name_count >= MAX_NODES) { fprintf(stderr, "too many nodes\n"); exit(1); }
    strncpy(names[name_count], s, NAME_LEN-1);
    names[name_count][NAME_LEN-1] = '\0';
    return name_count++;
}

static int find_node(const char *s) {
    for (int i = 0; i < name_count; i++)
        if (strcmp(names[i], s) == 0) return i;
    return -1;
}

/* ---- adjacency list ---- */
static int adj[MAX_NODES][64];   /* adj[u] = list of successors */
static int deg[MAX_NODES];

static void add_edge(int u, int v) {
    if (deg[u] >= 64) { fprintf(stderr, "degree too high\n"); exit(1); }
    adj[u][deg[u]++] = v;
}

/* ---- parse input ---- */
static void parse(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror(filename); exit(1); }
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        /* format: name: n1 n2 ... */
        char *colon = strchr(line, ':');
        if (!colon) continue;
        *colon = '\0';
        /* trim */
        char src[NAME_LEN];
        sscanf(line, "%s", src);
        int u = find_or_add(src);
        char *rest = colon + 1;
        char tok[NAME_LEN];
        while (sscanf(rest, "%s", tok) == 1) {
            int v = find_or_add(tok);
            add_edge(u, v);
            /* advance past this token */
            rest += strspn(rest, " \t\r\n");
            rest += strlen(tok);
        }
    }
    fclose(f);
}

/* ---- Part 1: count paths from 'you' to 'out' via DP ----
   dp[node] = number of paths from node to 'out'
   We process in reverse topological order (since it's a DAG).
   Simple memoisation with DFS. */

static long long dp1[MAX_NODES];
static int8_t    vis1[MAX_NODES];  /* 0=unvisited,1=in-stack(cycle!),2=done */

static int OUT_ID = -1;

static long long count_paths(int u) {
    if (vis1[u] == 2) return dp1[u];
    if (vis1[u] == 1) { fprintf(stderr,"cycle at %s\n",names[u]); return 0; }
    vis1[u] = 1;
    if (u == OUT_ID) {
        dp1[u] = 1;
    } else {
        long long sum = 0;
        for (int i = 0; i < deg[u]; i++)
            sum += count_paths(adj[u][i]);
        dp1[u] = sum;
    }
    vis1[u] = 2;
    return dp1[u];
}

/* ---- Part 2: count paths from 'svr' to 'out' visiting both dac and fft ----
   State: (node, mask) where mask has bits:
     bit 0 = have visited dac
     bit 1 = have visited fft
   dp2[node][mask] = number of paths from node to out
                      such that, combined with 'mask' already seen,
                      the total path visits dac and fft
   
   Specifically: dp2[node][mask] = number of paths from node to out
   where 'mask' is what has been visited SO FAR (before reaching 'node',
   but we also check if 'node' itself is dac/fft to update mask).
   
   Actually easier: dp2[node][mask] = # of paths from 'node' to out
   that additionally visit the set (full_mask ^ mask) among {dac,fft}
   that haven't been visited yet.
   
   Let me define: dp2[node][mask] = # paths from node to out,
   where mask is the set already collected ENTERING node.
   
   When we enter node, if node==dac add bit0, if node==fft add bit1.
   new_mask = mask | (node==dac ? 1 : 0) | (node==fft ? 2 : 0)
   if node==out: return (new_mask == 3) ? 1 : 0
   else: sum over successors of dp2[succ][new_mask]
   
   Memoize on (node, mask). mask in 0..3.
*/

static long long dp2[MAX_NODES][4];
static int8_t    vis2[MAX_NODES][4];
static int DAC_ID = -1, FFT_ID = -1;

static long long count_paths2(int u, int mask) {
    /* update mask for current node */
    if (u == DAC_ID) mask |= 1;
    if (u == FFT_ID) mask |= 2;

    if (vis2[u][mask] == 2) return dp2[u][mask];
    if (vis2[u][mask] == 1) return 0; /* cycle */
    vis2[u][mask] = 1;

    long long val;
    if (u == OUT_ID) {
        val = (mask == 3) ? 1 : 0;
    } else {
        long long sum = 0;
        for (int i = 0; i < deg[u]; i++)
            sum += count_paths2(adj[u][i], mask);
        val = sum;
    }
    dp2[u][mask] = val;
    vis2[u][mask] = 2;
    return val;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc >= 2) part = atoi(argv[1]);

    memset(deg, 0, sizeof(deg));
    memset(dp1, 0, sizeof(dp1));
    memset(vis1, 0, sizeof(vis1));
    memset(dp2, 0, sizeof(dp2));
    memset(vis2, 0, sizeof(vis2));

    parse("example2.txt");

    OUT_ID = find_node("out");
    DAC_ID = find_node("dac");
    FFT_ID = find_node("fft");
    int YOU_ID = find_node("you");
    int SVR_ID = find_node("svr");

    if (OUT_ID < 0) { fprintf(stderr, "'out' not found\n"); return 1; }

    long long ans1 = 0, ans2 = 0;

    if (part == 0 || part == 1) {
        if (YOU_ID < 0) { fprintf(stderr, "'you' not found\n"); return 1; }
        ans1 = count_paths(YOU_ID);
    }

    if (part == 0 || part == 2) {
        if (SVR_ID < 0) { fprintf(stderr, "'svr' not found\n"); return 1; }
        if (DAC_ID < 0) { fprintf(stderr, "'dac' not found\n"); return 1; }
        if (FFT_ID < 0) { fprintf(stderr, "'fft' not found\n"); return 1; }
        ans2 = count_paths2(SVR_ID, 0);
    }

    if (part == 0 || part == 1)
        printf("Part 1: %lld\n", ans1);
    if (part == 0 || part == 2)
        printf("Part 2: %lld\n", ans2);

    return 0;
}
