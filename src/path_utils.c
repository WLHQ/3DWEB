#include "path_utils.h"
#include <string.h>

bool contains_path_traversal(const char *path)
{
    if (!path) return false;

    // Check for ".."
    const char *p = path;
    while ((p = strstr(p, "..")) != NULL) {
        // Check if ".." is a path component
        // It is a component if:
        // 1. It is at the start (p == path) OR preceded by '/'
        // 2. It is at the end (p[2] == '\0') OR followed by '/'

        bool start_ok = (p == path) || (p[-1] == '/');
        bool end_ok = (p[2] == '\0') || (p[2] == '/');

        if (start_ok && end_ok) {
            return true;
        }

        // Advance to avoid infinite loop
        p += 2;
    }

    return false;
}
