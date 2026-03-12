#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Include mocks so utils.h can compile
#include "mocks/3ds.h"

// Include the header with the new optimization
#include "utils.h"

// The old implementation (simulated as non-inline for fair comparison)
__attribute__((noinline))
int startWith_old(char *str, char *start)
{
    if (!str || !start)
        return (0);
    return strncmp(str, start, strlen(start)) == 0;
}

void verify(char *str, char *start, int expected) {
    int res_old = startWith_old(str, start);
    int res_new = startWith(str, start); // Uses header implementation

    if (res_old != expected) {
        printf("FAILED (Old): str='%s', start='%s', expected=%d, got=%d\n", str ? str : "NULL", start ? start : "NULL", expected, res_old);
        exit(1);
    }
    if (res_new != expected) {
        printf("FAILED (New): str='%s', start='%s', expected=%d, got=%d\n", str ? str : "NULL", start ? start : "NULL", expected, res_new);
        exit(1);
    }
}

void benchmark(int iterations) {
    char *str = malloc(100);
    if (!str) return;
    strcpy(str, "GET /index.html HTTP/1.1");

    clock_t begin, end;
    double time_spent;

    printf("Benchmark: iterations=%d\n", iterations);

    // Old Implementation
    begin = clock();
    for (int i = 0; i < iterations; i++) {
        volatile int res = startWith_old(str, "GET");
        volatile int res2 = startWith_old(str, "POST"); // mismatch case
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("  Old (noinline)      : %.6f s\n", time_spent);

    // New Implementation (from utils.h)
    begin = clock();
    for (int i = 0; i < iterations; i++) {
        volatile int res = startWith(str, "GET");
        volatile int res2 = startWith(str, "POST"); // mismatch case
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("  New (static inline) : %.6f s\n", time_spent);

    free(str);
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
    printf("All verification tests passed!\n\n");

    benchmark(100000000);
    return 0;
}
