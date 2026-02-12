#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Mock necessary types before including source if needed,
// but 3ds.h is mocked via -I path.

// Mock mem_utils functions
void *memalloc(size_t size) {
    void *ptr = calloc(1, size);
    return ptr;
}

void *memdup(const void *src, size_t size) {
    void *ptr = malloc(size);
    if (ptr) memcpy(ptr, src, size);
    return ptr;
}

void memdel(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

// Include the source file directly
#include "../src/favicon_handler.c"

// Definitions for externs in httpserver.h
PrintConsole topScreen;
PrintConsole bottomScreen;
LightLock printLock;
SystemConfig sys_conf;

int main() {
    printf("Running favicon_handler tests...\n");

    // Test 1: verify is_favicon_request
    http_request req;
    memset(&req, 0, sizeof(req));
    req.path = "/favicon.ico";
    assert(is_favicon_request(&req) == 1);

    req.path = "/index.html";
    assert(is_favicon_request(&req) == 0);

    // Test 2: verify get_favicon_icon returns SVG for standard user agent
    req.path = "/favicon.ico";
    req.agent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3";

    http_response *res = get_favicon_icon(&req);
    assert(res != NULL);
    assert(res->code == 200);
    assert(strstr(res->content_type, "image/svg+xml") != NULL);
    assert(res->payload != NULL);
    assert(strstr(res->payload, "<svg") != NULL);

    // Clean up response
    if (res->content_type) free(res->content_type);
    if (res->payload) free(res->payload);
    free(res);

    // Test 3: verify get_favicon_icon for legacy/unknown user agent
    // Since we forced the fallback to also send SVG, it should behave similarly.
    req.agent = "UnknownBrowser/1.0";
    res = get_favicon_icon(&req);
    assert(res != NULL);
    assert(res->code == 200);
    assert(strstr(res->content_type, "image/svg+xml") != NULL);
    assert(res->payload != NULL);
    assert(strstr(res->payload, "<svg") != NULL);

    // Clean up response
    if (res->content_type) free(res->content_type);
    if (res->payload) free(res->payload);
    free(res);

    // Test 4: verify with NULL agent
    req.agent = NULL;
    res = get_favicon_icon(&req);
    assert(res != NULL);
    assert(res->code == 200);
    assert(strstr(res->content_type, "image/svg+xml") != NULL);

    if (res->content_type) free(res->content_type);
    if (res->payload) free(res->payload);
    free(res);

    printf("All tests passed!\n");
    return 0;
}
