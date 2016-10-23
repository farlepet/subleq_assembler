#ifndef DEFINITIONS_H
#define DEFINITIONS_H

struct definition {
    char name[32];
    int64_t value;
};

extern struct definition *definitions;

void def_create(char *line);
struct definition *find_definition(char *name);

#endif
