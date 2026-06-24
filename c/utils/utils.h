#ifndef UTILS_H
#define UTILS_H

/* Reads an entire file into a malloc'd, null-terminated buffer. Exits on
 * failure to open or allocate. Caller must free() the returned pointer. */
char *read_file(const char *path);

#endif /* UTILS_H */
