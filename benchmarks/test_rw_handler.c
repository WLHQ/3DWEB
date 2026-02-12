#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Include mocks first to ensure types are defined
#include "mocks/3ds.h"

// Mock for startWith (used by rw_handler.c)
// utils.h signature: int startWith(char *str, char *start);
int startWith(char *str, char *start) {
    if (!str || !start) return 0;
    return strncmp(str, start, strlen(start)) == 0;
}

// Mock for memalloc
void* memalloc(size_t size) {
    return calloc(1, size);
}

// Mock for memdup (used in rw_handler.c)
void *memdup(const void *src, size_t size) {
    void *dest = malloc(size);
    if (dest) memcpy(dest, src, size);
    return dest;
}

// Mock for memdel
void memdel(void **data) {
    if (data && *data) {
        free(*data);
        *data = NULL;
    }
}

// Mock for printTop (variadic)
// utils.h signature: int printTop(const char *format, ...);
int printTop(const char *fmt, ...) {
    return 0;
}

// Mock for printBottom (variadic)
// utils.h signature: int printBottom(const char *format, ...);
int printBottom(const char *fmt, ...) {
    return 0;
}

// Mock for clearBottom
void clearBottom() {}

// Mock for failExit
void failExit(const char *fmt, ...) { exit(1); }

// Include the source file to verify is_read_request
// We use a relative path.
#include "../src/rw_handler.c"

void test_is_read_request() {
    printf("Testing is_read_request...\n");

    // Helper to run test
    struct {
        const char *path;
        int expected;
    } cases[] = {
        {"/readmem/1000/10", 1},
        {"/readmem/ABC/20", 1},
        {"/readmem/FFFFFFFF/10", 0}, // invalid addr (0xFFFFFFFF)
        {"/readmem/0/10", 0},        // invalid addr (0)
        {"/readmem/1000/0", 0},      // invalid len (<=0)
        {"/readmem/1000/300", 0},    // invalid len (>256)
        {"/readmem/1000", 0},        // missing param
        {"/readmem/1000/10/extra", 0}, // extra param
        {"/other/1000/10", 0},       // wrong prefix
        {"/readmem/", 0},            // empty
    };

    int failed = 0;
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        http_request req;
        memset(&req, 0, sizeof(req));
        req.path = (char*)cases[i].path; // startWith takes char* but doesn't modify it hopefully

        int result = is_read_request(&req);

        if (result != cases[i].expected) {
            printf("FAIL: path='%s', expected=%d, got=%d\n", cases[i].path, cases[i].expected, result);
            failed = 1;
        }
    }

    if (!failed) {
        printf("PASS: is_read_request\n");
    } else {
        printf("FAIL: is_read_request\n");
        exit(1);
    }
}

int main() {
    test_is_read_request();
    return 0;
}
