#ifndef WRITE_H
#define WRITE_H

#include <stdint.h>

void write_op(FILE *, int64_t, int64_t, int64_t, int, int);
void write_word(FILE *, int64_t, int, int);

#endif
