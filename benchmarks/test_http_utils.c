#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http_utils.h"

// Minimal implementations for other utils if needed
int printTop(const char *format, ...) { return 0; }
int printBottom(const char *format, ...) { return 0; }
void clearBottom() {}
void failExit(const char *fmt, ...) { exit(1); }

void test_get_request_name() {
    printf("Testing get_request_name...\n");

    struct {
        http_request_type type;
        const char *expected;
    } cases[] = {
        {GET, "GET"},
        {HEAD, "HEAD"},
        {POST, "POST"},
        {OPTIONS, "OPTIONS"},
        {CONNECT, "CONNECT"},
        {TRACE, "TRACE"},
        {PUT, "PUT"},
        {PATCH, "PATCH"},
        {DELETE, "DELETE"},
        {UNKNOWN, NULL} // Depending on implementation, default might return NULL
    };

    int failed = 0;
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        char *result = get_request_name(cases[i].type);
        if (result == NULL && cases[i].expected == NULL) {
            // pass
        } else if (result == NULL || cases[i].expected == NULL || strcmp(result, cases[i].expected) != 0) {
            printf("FAIL: type=%d, expected=%s, got=%s\n", cases[i].type, cases[i].expected ? cases[i].expected : "NULL", result ? result : "NULL");
            failed = 1;
        }
    }

    if (!failed) {
        printf("PASS: get_request_name\n");
    } else {
        printf("FAIL: get_request_name\n");
        exit(1);
    }
}

void test_get_type() {
    printf("Testing get_type...\n");

    struct {
        char *input;
        http_request_type expected;
    } cases[] = {
        {"GET / HTTP/1.1", GET},
        {"HEAD / HTTP/1.1", HEAD},
        {"POST /api/data", POST},
        {"OPTIONS * HTTP/1.1", OPTIONS},
        {"CONNECT google.com:443", CONNECT},
        {"TRACE /", TRACE},
        {"PUT /upload", PUT},
        {"PATCH /update", PATCH},
        {"DELETE /resource", DELETE},
        {"INVALID request", UNKNOWN},
        {NULL, UNKNOWN} // startWith handles NULL check
    };

    int failed = 0;
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        // We need to be careful with NULL input if startWith handles it.
        // In http_utils.c: get_type calls startWith(str, "GET") etc.
        // startWith handles NULL by returning 0.
        // So get_type(NULL) should return UNKNOWN (by falling through).
        // However, in get_type:
        // if (startWith(str, "GET")) ...
        // If str is NULL, startWith returns 0.
        // It falls through all ifs and returns UNKNOWN.

        http_request_type result = get_type(cases[i].input);
        if (result != cases[i].expected) {
            printf("FAIL: input='%s', expected=%d, got=%d\n", cases[i].input ? cases[i].input : "NULL", cases[i].expected, result);
            failed = 1;
        }
    }

    if (!failed) {
        printf("PASS: get_type\n");
    } else {
        printf("FAIL: get_type\n");
        exit(1);
    }
}

int main() {
    test_get_request_name();
    test_get_type();
    printf("All tests passed!\n");
    return 0;
}
