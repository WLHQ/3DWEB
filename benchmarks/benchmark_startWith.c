#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int startWith_original(char *str, char *start)
{
    if (!str || !start)
        return (0);
    int startlen = strlen(start);
    return startlen <= strlen(str) && strncmp(str, start, startlen) == 0;
}

int startWith_optimized(char *str, char *start)
{
    if (!str || !start)
        return (0);
    size_t startlen = strlen(start); // Should probably use size_t
    return strncmp(str, start, startlen) == 0;
}

void verify(char *str, char *start, int expected) {
    int res_orig = startWith_original(str, start);
    int res_opt = startWith_optimized(str, start);

    if (res_orig != expected) {
        printf("FAILED (Original): str='%s', start='%s', expected=%d, got=%d\n", str ? str : "NULL", start ? start : "NULL", expected, res_orig);
        exit(1);
    }
    if (res_opt != expected) {
        printf("FAILED (Optimized): str='%s', start='%s', expected=%d, got=%d\n", str ? str : "NULL", start ? start : "NULL", expected, res_opt);
        exit(1);
    }
    // printf("PASS: str='%s', start='%s'\n", str ? str : "NULL", start ? start : "NULL");
}

void benchmark(int iterations, int str_len, int start_len) {
    char *str = malloc(str_len + 1);
    char *start = malloc(start_len + 1);

    // Initialize str with 'a's
    memset(str, 'a', str_len);
    str[str_len] = '\0';

    // Initialize start with 'a's
    memset(start, 'a', start_len);
    start[start_len] = '\0';

    clock_t begin, end;
    double time_spent;

    // Original
    begin = clock();
    for (int i = 0; i < iterations; i++) {
        volatile int res = startWith_original(str, start);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Original (str_len=%d, start_len=%d): %.6f seconds\n", str_len, start_len, time_spent);

    // Optimized
    begin = clock();
    for (int i = 0; i < iterations; i++) {
        volatile int res = startWith_optimized(str, start);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Optimized (str_len=%d, start_len=%d): %.6f seconds\n", str_len, start_len, time_spent);

    free(str);
    free(start);
}

int main() {
    printf("Verifying correctness...\n");
    verify("hello", "he", 1);
    verify("hello", "hello", 1);
    verify("hello", "helloo", 0);
    verify("hello", "", 1);
    verify("", "", 1);
    verify(NULL, "a", 0);
    verify("a", NULL, 0);
    verify("short", "loooong", 0);
    printf("All tests passed!\n\n");

    printf("Benchmarking...\n");
    int iterations = 10000000;

    // Case 1: Short strings
    benchmark(iterations, 10, 5);

    // Case 2: Long str, short start (Should show big improvement)
    benchmark(iterations, 10000, 5);

    // Case 3: Long str, long start
    benchmark(iterations, 10000, 5000);

    return 0;
}
