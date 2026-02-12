#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Simulated memalloc from src/mem_utils.c
void *simulated_memalloc(size_t size) {
    void *res = malloc(size);
    if (res)
        memset(res, 0, size);
    return res;
}

// Case 1: Redundant memset (Current implementation)
void redundant_allocation(size_t size) {
    char *payload = simulated_memalloc(size);
    if (!payload) return;

    // The redundant memset
    memset(payload, 0, size);

    free(payload);
}

// Case 2: Optimized allocation (Desired implementation)
void optimized_allocation(size_t size) {
    char *payload = simulated_memalloc(size);
    if (!payload) return;

    // No redundant memset

    free(payload);
}

void benchmark(int iterations, size_t size) {
    clock_t begin, end;
    double time_spent;

    printf("Benchmarking allocation of size %zu bytes for %d iterations...\n", size, iterations);

    // Baseline (Redundant)
    begin = clock();
    for (int i = 0; i < iterations; i++) {
        redundant_allocation(size);
    }
    end = clock();
    double baseline_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Baseline (Redundant): %.6f seconds\n", baseline_time);

    // Optimized
    begin = clock();
    for (int i = 0; i < iterations; i++) {
        optimized_allocation(size);
    }
    end = clock();
    double optimized_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Optimized:            %.6f seconds\n", optimized_time);

    if (baseline_time > 0) {
        double improvement = ((baseline_time - optimized_time) / baseline_time) * 100.0;
        printf("Improvement:          %.2f%%\n", improvement);
    }
    printf("\n");
}

int main() {
    // 4098 bytes as used in connection.c
    benchmark(1000000, 4098);

    // Larger size to see impact on larger buffers (optional but interesting)
    benchmark(100000, 1024 * 1024); // 1MB

    return 0;
}
