#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Include source directly to avoid linking issues for standalone test
// This is a common trick for unit testing static functions or simple modules
// without complex build setups.
#include "../src/path_utils.c"

int failures = 0;

void test(const char *path, bool expected) {
    bool result = contains_path_traversal(path);
    if (result == expected) {
        printf("PASS: \"%s\" -> %s\n", path ? path : "NULL", result ? "true" : "false");
    } else {
        printf("FAIL: \"%s\" -> expected %s, got %s\n", path ? path : "NULL", expected ? "true" : "false", result ? "true" : "false");
        failures++;
    }
}

int main() {
    printf("Running path traversal tests...\n");

    // Safe paths
    test("/foo/bar", false);
    test("/foo/bar.txt", false);
    test("foo", false);
    test("/", false);
    test("", false);
    test(NULL, false);

    // Filenames containing dots but not traversal
    test("/foo..bar", false);
    test("/foo/..bar", false);
    test("/foo/bar..", false);
    test("/.../bar", false);
    test("/..../bar", false);
    test("..foo", false);
    test("foo..", false);

    // Unsafe paths
    test("/foo/../bar", true);
    test("../foo", true);
    test("foo/..", true);
    test("..", true);
    test("/..", true);
    test("/a/b/c/../../d", true);
    test("/../", true);

    if (failures == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("%d tests failed!\n", failures);
        return 1;
    }
}
