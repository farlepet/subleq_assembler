#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <definitions.h>
#include <values.h>

static int max_defs = 1024;
static int n_defs = 0;

struct definition *definitions = NULL;

void def_create(char *line) {
    if(definitions == NULL) definitions = (struct definition *)malloc(max_defs);
    if(n_defs == max_defs) {
        printf("def_create: Maximum number of definitions reached!\n");
        return;
    }
    char *name = strtok(line, ", ");
    char *value = strtok(NULL, "");
    if(!name || !value) {
        printf("def_create: incorrect definition statement!\n");
        return;
    }
    strcpy(definitions[n_defs].name, name);
    definitions[n_defs].value = get_number(value);
    //printf("DEF[%d]: %s[%04lX]\n", n_defs, definitions[n_defs].name, definitions[n_defs].value);
    n_defs++;
}

struct definition *find_definition(char *name) {
    int i = 0;
    for(; i < n_defs; i++) {
        if(!strcmp(definitions[i].name, name)) return &definitions[i];
    }
    return NULL;
}
