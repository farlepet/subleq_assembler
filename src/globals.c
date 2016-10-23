#include <stdio.h>

#include <globals.h>

FILE *in  = NULL, *out = NULL, *dbg = NULL;


uint64_t ENTRYPOINT = 0;
int endian = 0, bytes = 2, loc = 0;

uint32_t line = 0;
