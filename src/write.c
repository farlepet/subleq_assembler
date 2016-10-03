#include <byteswap.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void write_op(FILE *f, int64_t a, int64_t b, int64_t c, int endian, int size) {
    if(size < 1 || size > 8) {
        printf("write_op: INVALID WRITE SIZE: %d Must be between 1 and 8, inclusive!\n", size);
        exit(1);
    }

    void *A = &a;
    void *B = &b;
    void *C = &c;

    if(endian) { // Swap endianness
        a = __bswap_64(a);
        b = __bswap_64(b);
        c = __bswap_64(c);

        // Account for change in position for integers smaller then 64 bits
        A = A + (8 - size);
        B = B + (8 - size);
        C = C + (8 - size);
    }

    // Write integers to file
    fwrite(A, size, 1, f);
    fwrite(B, size, 1, f);
    fwrite(C, size, 1, f);
}

void write_word(FILE *f, int64_t w, int endian, int size) {
    if(size < 1 || size > 8) {
        printf("write_op: INVALID WRITE SIZE: %d Must be between 1 and 8, inclusive!\n", size);
        exit(1);
    }

    void *W = &w;
    
    if(endian) { // Swap endianness
        w = __bswap_64(w);

        // Account for change in position for integers smaller then 64 bits
        W = W + (8 - size);
    }

    // Write integer to file
    fwrite(W, size, 1, f);
}
