#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

extern FILE *in;
extern FILE *out;
extern FILE *dbg;

extern uint64_t ENTRYPOINT;
extern int endian;
extern int bytes;
extern int loc;

extern uint32_t line;

#endif
