#ifndef LABELS_H
#define LABELS_H

#include <stdint.h>

struct label {
	char     name[32];
	uint32_t hash;
	int64_t addr;
	uint32_t line;
};

void find_labels();
int find_label(char *name);

extern struct label *labels;

#endif
