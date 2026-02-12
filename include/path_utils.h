#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <stdbool.h>

/**
 * Checks if the given path contains directory traversal sequences ("..").
 *
 * @param path The path string to check.
 * @return true if the path contains traversal sequences, false otherwise.
 */
bool contains_path_traversal(const char *path);

#endif
