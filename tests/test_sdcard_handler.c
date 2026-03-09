#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

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

#include "3ds.h"

// Mock url_decode to avoid including the whole utils.c
void url_decode(char *dest, const char *src) {
    strcpy(dest, src);
}

bool contains_path_traversal(const char *path) {
    return strstr(path, "..") != NULL;
}

const char *get_mime_type(const char *path) {
    return "text/plain";
}

// Include the source file directly
#include "../src/sdcard_handler.c"

int main() {
    printf("Running sdcard_handler tests...\n");

    // 1. Create a temporary directory with many files
    const char *dir_path = "tests/test_data/large_dir";
    system("mkdir -p tests/test_data/large_dir");

    for (int i = 0; i < 500; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/test_file_with_a_long_name_to_fill_buffer_%03d.txt", dir_path, i);
        FILE *f = fopen(filepath, "w");
        if (f) fclose(f);
    }

    // 2. Call handle_directory_listing on the directory
    // This used to cause a buffer overflow or very slow performance due to O(N^2) strcat
    printf("Testing handle_directory_listing on large directory...\n");
    http_response *res = handle_directory_listing(dir_path);

    assert(res != NULL);
    assert(res->code == 200);
    assert(res->payload != NULL);
    assert(res->payload_len > 0);
    assert(res->payload_len <= 16384);

    // Check if the payload ends with ']'
    char *payload_str = (char *)res->payload;
    assert(payload_str[res->payload_len - 1] == ']');
    assert(payload_str[0] == '[');

    // Clean up response
    if (res->content_type) free(res->content_type);
    if (res->payload) free(res->payload);
    free(res);

    // 3. Clean up the temporary directory
    system("rm -rf tests/test_data/large_dir");

    printf("All sdcard_handler tests passed!\n");
    return 0;
}
