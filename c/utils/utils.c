#include "utils/utils.h"

#include <stdio.h>
#include <stdlib.h>

char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "error: could not open %s\n", path);
        exit(1);
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "error: could not seek %s\n", path);
        exit(1);
    }
    long size = ftell(f);
    if (size < 0) {
        fprintf(stderr, "error: could not determine size of %s\n", path);
        exit(1);
    }
    rewind(f);

    char *buf = malloc((size_t)size + 1);
    if (!buf) {
        fprintf(stderr, "error: out of memory reading %s\n", path);
        exit(1);
    }

    size_t read_count = fread(buf, 1, (size_t)size, f);
    buf[read_count] = '\0';
    fclose(f);
    return buf;
}
