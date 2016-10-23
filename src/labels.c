#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include <labels.h>
#include <values.h>
#include <globals.h>
#include <definitions.h>

struct label *labels;
static int max_labels = 1024; // Maximum number of lables allowed

void find_labels() {
	labels = malloc(sizeof(struct label) * max_labels);

	uint32_t line = 1;
	int64_t addr = ENTRYPOINT; // Place it above the device-mapped memory
	int c_idx = 0;

	char str[1024];

	while(fgets(str, 1024, in)) {
		// Convert entire line to uppercase
		unsigned i = 0;
		int not_white = 0, is_comment = 0, colon = 0;
		for(; i < strlen(str); i++) {
			str[i] = toupper(str[i]);
			if(str[i] == ':' && !is_comment) colon = i; // If it is the first char on the line, ignore it
			if(!not_white && str[i] == '#') is_comment = 1;
			if(!isspace(str[i])) not_white = 1;
		}

		if(colon) { // We found a label
            sprintf(labels[c_idx].name, "%.*s", colon, str);
			labels[c_idx].hash = strhash(labels[c_idx].name);
			labels[c_idx].line = line;
			labels[c_idx].addr = addr;
			c_idx++;
			continue;
		}

		line++;

		if(str[0] == '#' || strlen(str) == 0 ||
			!not_white || is_comment) continue; // This line is a comment, or whitespace

		if(str[0] == '.') { // Data
            if(strstr(str, ".RESV") == str) { // Reserve words
                char *val = str + 5;
                while(isspace(*val)) val++;
                addr += get_number(val);
                continue;
            }
            if(strstr(str, ".DEFN") == str) { // Define value
                char *val = str + 5;
                while(isspace(*val)) val++;
                def_create(val);
                continue;
            }

            char *dat = get_csv_val(str + 1);
            do {
                int i = 1;
                while(dat[i] == ' ') i++;
			    if(dat[i] == '"') {
				    i++;
				    while(dat[i++] != '"') addr++;
			    }
			    else addr++;
            } while((dat = get_csv_val(NULL)) != NULL);
			continue;
		}
        loc++;

		addr += 3;
	}

}

int find_label(char *name)
{
	int i = 0;
	
	uint32_t hash = strhash(name);

	for(; i < max_labels; i++) {
		if(labels[i].hash == hash)
			if(!strcmp(labels[i].name, name)) { // Just incase two labels have the same hash
				return i;
            }
	}
	return -1; // Not found...
}
