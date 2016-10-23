#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include <values.h>
#include <labels.h>
#include <globals.h>
#include <definitions.h>

char *get_csv_val(char *str) {
    static char *curr_str;
    if(str != NULL) curr_str = str;
    if(curr_str == NULL) return NULL;
    str = curr_str;
    while(*str != '\0' && *str != ',') {
        if(*str == '"') {
            str++;
            while(*str != '"') {
                if(*str == '\0') {
                    fprintf(stderr, "get_csv_val: Could not find ending quotation mark.");
                    return NULL;
                }
                str++;
            }
        }
        str++;
    }
    char *ret = curr_str;
    if(*str == '\0') curr_str = NULL;
    else curr_str = str + 1;
    *str = '\0';
    return ret;
}


uint32_t strhash(char *str) {
	uint32_t hash = 0;
	while(*str) {
		uint8_t end = ((hash >> 24) & 0xFF);
		hash = (hash << 8) | (*str ^ end);
		str++;
	}
	return hash;
}




int64_t get_number(char *str)
{
	while(isspace(*str)) str++;
    if(!isdigit(*str) && (*str != '+' && *str != '-')) {
		uint64_t i = 0;
		int64_t off = 0; // Signed, because it can be a negative offset

		for(; i < strlen(str); i++) {
			if(str[i] == '$') { // Used to represent an array index
				str[i++] = '\0';
				off = get_number(&str[i]);
			}
            if(str[i] == '\n' || str[i] == ' ') {
                str[i] = '\0';
                break;
            }
		}
        struct definition *def = find_definition(str);
        if(def == NULL) {
		    int idx = find_label(str);
		    if(idx == -1) {
		    	printf("ERROR: [line %d] Could not find label: '%s'", line, str);
		    	exit(1);
		    }
		    return labels[idx].addr + off;
        } else {
            return def->value + off;
        }
	}

	return strtoll(str, NULL, 0);
}

