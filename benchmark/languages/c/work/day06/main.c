#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WIDTH 200000

int main(int argc, char *argv[]) {
    int do_part1 = 1, do_part2 = 1;
    if (argc >= 2) {
        if (argv[1][0] == '1') do_part2 = 0;
        else if (argv[1][0] == '2') do_part1 = 0;
    }

    FILE *f = fopen("input.txt", "r");
    if (!f) { fprintf(stderr, "Cannot open input.txt\n"); return 1; }

    // Read all lines
    char **lines = NULL;
    int nlines = 0;
    int capacity = 0;
    char *buf = malloc(MAX_WIDTH + 2);

    while (fgets(buf, MAX_WIDTH + 1, f)) {
        // Remove trailing newline/CR
        int len = (int)strlen(buf);
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
            buf[--len] = '\0';
        }
        if (nlines >= capacity) {
            capacity = capacity ? capacity * 2 : 8;
            lines = realloc(lines, capacity * sizeof(char*));
        }
        lines[nlines] = malloc(len + 1);
        memcpy(lines[nlines], buf, len + 1);
        nlines++;
    }
    free(buf);
    fclose(f);

    // Strip trailing empty lines
    while (nlines > 0 && lines[nlines-1][0] == '\0') {
        free(lines[nlines-1]);
        nlines--;
    }

    if (nlines == 0) {
        if (do_part1) printf("Part 1: 0\n");
        if (do_part2) printf("Part 2: 0\n");
        free(lines);
        return 0;
    }

    // Find max width
    int width = 0;
    for (int i = 0; i < nlines; i++) {
        int len = (int)strlen(lines[i]);
        if (len > width) width = len;
    }

    // The last row is the operator row
    int op_row = nlines - 1;

    // Identify separator columns: columns where ALL rows have ' ' or are out of bounds
    int *is_sep = calloc(width, sizeof(int));
    for (int col = 0; col < width; col++) {
        int sep = 1;
        for (int row = 0; row < nlines; row++) {
            int rowlen = (int)strlen(lines[row]);
            char c = (col < rowlen) ? lines[row][col] : ' ';
            if (c != ' ') { sep = 0; break; }
        }
        is_sep[col] = sep;
    }

    // Find problem groups: ranges [start, end) of consecutive non-separator columns
    typedef struct { int start; int end; } Problem;
    Problem *probs = NULL;
    int nprobs = 0;
    int pcap = 0;

    int in_prob = 0;
    int prob_start = 0;
    for (int col = 0; col <= width; col++) {
        int sep = (col == width) ? 1 : is_sep[col];
        if (!sep && !in_prob) {
            in_prob = 1;
            prob_start = col;
        } else if (sep && in_prob) {
            in_prob = 0;
            if (nprobs >= pcap) {
                pcap = pcap ? pcap * 2 : 64;
                probs = realloc(probs, pcap * sizeof(Problem));
            }
            probs[nprobs].start = prob_start;
            probs[nprobs].end = col;
            nprobs++;
        }
    }

    // --- Part 1 ---
    // Within each problem: each data row (0..op_row-1) gives one number
    // by concatenating non-space chars across all columns of the problem.
    // Operator is in op_row (any column of the problem that has '*' or '+').
    long long grand_total_1 = 0;
    if (do_part1) {
        for (int p = 0; p < nprobs; p++) {
            int s = probs[p].start;
            int e = probs[p].end;

            // Find operator
            char op = ' ';
            for (int col = s; col < e; col++) {
                int rowlen = (int)strlen(lines[op_row]);
                char c = (col < rowlen) ? lines[op_row][col] : ' ';
                if (c == '*' || c == '+') { op = c; break; }
            }
            if (op == ' ') continue;

            // For each data row, collect digits across columns s..e-1
            long long result = 0;
            int first = 1;
            for (int row = 0; row < op_row; row++) {
                int rowlen = (int)strlen(lines[row]);
                char numbuf[64];
                int nlen = 0;
                for (int col = s; col < e && nlen < 63; col++) {
                    char c = (col < rowlen) ? lines[row][col] : ' ';
                    if (c >= '0' && c <= '9') {
                        numbuf[nlen++] = c;
                    }
                }
                if (nlen == 0) continue;
                numbuf[nlen] = '\0';
                long long val = atoll(numbuf);
                if (first) {
                    result = val;
                    first = 0;
                } else {
                    if (op == '*') result *= val;
                    else result += val;
                }
            }
            grand_total_1 += result;
        }
    }

    // --- Part 2 ---
    // Within each problem: each column gives one number
    // by concatenating non-space chars from top to bottom (data rows only).
    // Operator is in the leftmost column of the problem (op_row).
    // Problems are read right-to-left (but operator is in leftmost col).
    // Within a problem, columns are read right-to-left.
    long long grand_total_2 = 0;
    if (do_part2) {
        // Process problems in any order (grand total is sum), but columns within problem right-to-left
        for (int p = 0; p < nprobs; p++) {
            int s = probs[p].start;
            int e = probs[p].end;

            // Find operator: in op_row, leftmost column of the problem
            char op = ' ';
            for (int col = s; col < e; col++) {
                int rowlen = (int)strlen(lines[op_row]);
                char c = (col < rowlen) ? lines[op_row][col] : ' ';
                if (c == '*' || c == '+') { op = c; break; }
            }
            if (op == ' ') continue;

            // Read columns from right (e-1) to left (s), each column is a number
            long long result = 0;
            int first = 1;
            for (int col = e - 1; col >= s; col--) {
                char numbuf[64];
                int nlen = 0;
                for (int row = 0; row < op_row && nlen < 63; row++) {
                    int rowlen = (int)strlen(lines[row]);
                    char c = (col < rowlen) ? lines[row][col] : ' ';
                    if (c >= '0' && c <= '9') {
                        numbuf[nlen++] = c;
                    }
                }
                if (nlen == 0) continue;
                numbuf[nlen] = '\0';
                long long val = atoll(numbuf);
                if (first) {
                    result = val;
                    first = 0;
                } else {
                    if (op == '*') result *= val;
                    else result += val;
                }
            }
            grand_total_2 += result;
        }
    }

    if (do_part1) printf("Part 1: %lld\n", grand_total_1);
    if (do_part2) printf("Part 2: %lld\n", grand_total_2);

    // Cleanup
    free(is_sep);
    free(probs);
    for (int i = 0; i < nlines; i++) free(lines[i]);
    free(lines);

    return 0;
}
