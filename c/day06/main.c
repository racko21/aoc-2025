/* Day 6: Trash Compactor. A math worksheet has numbers stacked vertically,
 * one operator per problem on the bottom row. Tokenizing each line by
 * whitespace yields one column per problem (since columns of digits are
 * separated by runs of spaces); applying the bottom row's operator across
 * the numbers in that column gives each problem's result. Part 1 sums all
 * problem results. */
#define _POSIX_C_SOURCE 199309L
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils/utils.h"

typedef struct {
    char **tokens;
    int ntokens;
} TokenLine;

/* Splits a line into whitespace-separated tokens. Caller must free via
 * free_token_line(). */
static TokenLine tokenize_line(const char *line) {
    TokenLine tl;
    int cap = 8;
    tl.tokens = malloc((size_t)cap * sizeof(char *));
    tl.ntokens = 0;

    const char *p = line;
    while (*p != '\0') {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        const char *start = p;
        while (*p != '\0' && *p != ' ' && *p != '\t') p++;
        size_t len = (size_t)(p - start);

        if (tl.ntokens == cap) {
            cap *= 2;
            tl.tokens = realloc(tl.tokens, (size_t)cap * sizeof(char *));
        }
        tl.tokens[tl.ntokens] = malloc(len + 1);
        memcpy(tl.tokens[tl.ntokens], start, len);
        tl.tokens[tl.ntokens][len] = '\0';
        tl.ntokens++;
    }

    return tl;
}

static void free_token_line(TokenLine *tl) {
    for (int i = 0; i < tl->ntokens; i++) free(tl->tokens[i]);
    free(tl->tokens);
}

/* Splits the file's text into trimmed, non-empty lines. Caller must free
 * each returned string and the array itself. */
static char **split_lines(char *text, int *nlines_out) {
    int cap = 8, n = 0;
    char **lines = malloc((size_t)cap * sizeof(char *));

    char *p = text;
    while (*p != '\0') {
        char *newline = strchr(p, '\n');
        size_t len = newline != NULL ? (size_t)(newline - p) : strlen(p);
        while (len > 0 && (p[len - 1] == '\r')) len--;

        if (len > 0) {
            if (n == cap) {
                cap *= 2;
                lines = realloc(lines, (size_t)cap * sizeof(char *));
            }
            lines[n] = malloc(len + 1);
            memcpy(lines[n], p, len);
            lines[n][len] = '\0';
            n++;
        }

        if (newline == NULL) break;
        p = newline + 1;
    }

    *nlines_out = n;
    return lines;
}

/* Solves Part 1: applies each problem's operator to its stacked numbers
 * and sums all problem results. */
long long solve_part1(const char *path) {
    char *text = read_file(path);

    int nlines = 0;
    char **raw_lines = split_lines(text, &nlines);

    TokenLine *lines = malloc((size_t)nlines * sizeof(TokenLine));
    for (int i = 0; i < nlines; i++) {
        lines[i] = tokenize_line(raw_lines[i]);
        free(raw_lines[i]);
    }
    free(raw_lines);

    int op_row = nlines - 1;
    int ncols = lines[op_row].ntokens;
    long long total = 0;

    for (int j = 0; j < ncols; j++) {
        char op = lines[op_row].tokens[j][0];
        long long acc = (op == '*') ? 1 : 0;
        for (int i = 0; i < op_row; i++) {
            long long val = atoll(lines[i].tokens[j]);
            if (op == '*') {
                acc *= val;
            } else {
                acc += val;
            }
        }
        total += acc;
    }

    for (int i = 0; i < nlines; i++) free_token_line(&lines[i]);
    free(lines);
    free(text);

    return total;
}

/* Solves Part 2: numbers are read column-wise instead of row-wise (most
 * significant digit on top, least significant on bottom). Columns that
 * are blank in every row, including the operator row, separate problems;
 * within a problem's columns, each column yields one operand and the
 * operator row gives the single operator to apply across all of them. */
long long solve_part2(const char *path) {
    char *text = read_file(path);

    int nlines = 0;
    char **raw_lines = split_lines(text, &nlines);

    int width = 0;
    for (int i = 0; i < nlines; i++) {
        int len = (int)strlen(raw_lines[i]);
        if (len > width) width = len;
    }

    char **lines = malloc((size_t)nlines * sizeof(char *));
    for (int i = 0; i < nlines; i++) {
        lines[i] = malloc((size_t)width + 1);
        int len = (int)strlen(raw_lines[i]);
        memcpy(lines[i], raw_lines[i], (size_t)len);
        for (int c = len; c < width; c++) lines[i][c] = ' ';
        lines[i][width] = '\0';
        free(raw_lines[i]);
    }
    free(raw_lines);

    int op_row = nlines - 1;
    long long total = 0;
    char digit_buf[64];

    int c = 0;
    while (c < width) {
        bool is_separator = true;
        for (int i = 0; i < nlines; i++) {
            if (lines[i][c] != ' ') {
                is_separator = false;
                break;
            }
        }
        if (is_separator) {
            c++;
            continue;
        }

        int start = c;
        while (c < width) {
            bool sep = true;
            for (int i = 0; i < nlines; i++) {
                if (lines[i][c] != ' ') {
                    sep = false;
                    break;
                }
            }
            if (sep) break;
            c++;
        }
        int end = c;

        char op = ' ';
        for (int col = start; col < end; col++) {
            if (lines[op_row][col] != ' ') {
                op = lines[op_row][col];
                break;
            }
        }

        long long acc = (op == '*') ? 1 : 0;
        for (int col = start; col < end; col++) {
            int dlen = 0;
            for (int row = 0; row < op_row; row++) {
                char ch = lines[row][col];
                if (ch != ' ') digit_buf[dlen++] = ch;
            }
            if (dlen == 0) continue;
            digit_buf[dlen] = '\0';
            long long val = atoll(digit_buf);
            if (op == '*') {
                acc *= val;
            } else {
                acc += val;
            }
        }
        total += acc;
    }

    for (int i = 0; i < nlines; i++) free(lines[i]);
    free(lines);
    free(text);

    return total;
}

#ifndef UNIT_TEST
static double elapsed_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000.0 + (b.tv_nsec - a.tv_nsec) / 1e6;
}

int main(int argc, char *argv[]) {
    int part = 0;
    if (argc > 1) part = atoi(argv[1]);

    struct timespec t0, t1;
    if (part == 0 || part == 1) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans1 = solve_part1("day06/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 1: %lld\n", ans1);
        fprintf(stderr, "runtime_ms_part1: %.2f\n", elapsed_ms(t0, t1));
    }
    if (part == 0 || part == 2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        long long ans2 = solve_part2("day06/input.txt");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Part 2: %lld\n", ans2);
        fprintf(stderr, "runtime_ms_part2: %.2f\n", elapsed_ms(t0, t1));
    }
    return 0;
}
#endif
