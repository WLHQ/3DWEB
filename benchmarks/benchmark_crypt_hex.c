#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Original decode_hex function
static unsigned char decode_hex_original(char c)
{
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= '0' && c <= '9')
		return c - '0';
	return 0; // pls
}

// Optimized lookup table
static unsigned char hex_lookup[256];

void init_hex_lookup() {
    memset(hex_lookup, 0, 256);
    for (int i = 0; i < 10; i++) hex_lookup['0' + i] = i;
    for (int i = 0; i < 6; i++) {
        hex_lookup['a' + i] = 10 + i;
        hex_lookup['A' + i] = 10 + i;
    }
}

// Function to decode using original method
void decode_loop_original(const char *p, unsigned char *inbuf, int l) {
    int ofs = 0;
    // int buf_l = l/2; // unused in this simple loop measure
    while (ofs < l)
    {
        inbuf[ofs/2] = (decode_hex_original(p[ofs]) << 4) | decode_hex_original(p[ofs+1]);
        ofs += 2;
    }
}

// Function to decode using optimized method
void decode_loop_optimized(const char *p, unsigned char *inbuf, int l) {
    int buf_l = l / 2;
    for (int i = 0; i < buf_l; i++) {
        inbuf[i] = (hex_lookup[(unsigned char)p[2*i]] << 4) | hex_lookup[(unsigned char)p[2*i+1]];
    }
}

// Function to verify correctness
void verify(const char *input, int len) {
    unsigned char *out1 = malloc(len / 2);
    unsigned char *out2 = malloc(len / 2);

    decode_loop_original(input, out1, len);
    decode_loop_optimized(input, out2, len);

    if (memcmp(out1, out2, len / 2) != 0) {
        printf("FAILED: Output mismatch!\n");
        printf("Input: %s\n", input);
        printf("Original: ");
        for(int i=0; i<len/2; i++) printf("%02x ", out1[i]);
        printf("\nOptimized: ");
        for(int i=0; i<len/2; i++) printf("%02x ", out2[i]);
        printf("\n");
        exit(1);
    }

    free(out1);
    free(out2);
}

void benchmark(int iterations, int str_len) {
    // Generate a random hex string
    char *input = malloc(str_len + 1);
    const char hex_chars[] = "0123456789ABCDEFabcdef";
    for (int i = 0; i < str_len; i++) {
        input[i] = hex_chars[rand() % (sizeof(hex_chars) - 1)];
    }
    input[str_len] = '\0';

    unsigned char *output = malloc(str_len / 2);

    clock_t begin, end;
    double time_original, time_optimized;

    // Benchmark Original
    begin = clock();
    for (int i = 0; i < iterations; i++) {
        decode_loop_original(input, output, str_len);
    }
    end = clock();
    time_original = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Original (len=%d): %.6f seconds\n", str_len, time_original);

    // Benchmark Optimized
    begin = clock();
    for (int i = 0; i < iterations; i++) {
        decode_loop_optimized(input, output, str_len);
    }
    end = clock();
    time_optimized = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Optimized (len=%d): %.6f seconds\n", str_len, time_optimized);

    printf("Improvement: %.2f%%\n", (time_original - time_optimized) / time_original * 100.0);

    free(input);
    free(output);
}

int main() {
    srand(time(NULL));
    init_hex_lookup();

    printf("Verifying correctness...\n");
    verify("0123456789ABCDEFabcdef", 22);
    verify("1a2b3c4d5e6f", 12);
    printf("Verification passed!\n\n");

    printf("Benchmarking...\n");
    // Short string
    benchmark(10000000, 32); // 32 chars = 16 bytes
    // Long string
    benchmark(1000000, 1024); // 1024 chars = 512 bytes

    return 0;
}
