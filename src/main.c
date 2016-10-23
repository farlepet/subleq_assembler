#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include <write.h>
#include <values.h>
#include <labels.h>
#include <globals.h>


static char *infile    = NULL;
static char *outfile   = NULL;
static char *debugfile = NULL;

void handle_opts(int, char **);
void usage(int);
void debug_write(int64_t, uint32_t);


int main(int argc, char **argv) {
	handle_opts(argc, argv);

	if(!infile || !outfile) usage(1);

	in  = fopen(infile, "r");
	out = fopen(outfile, "w");

	if(!in) {
		printf("Input file %s could not be opened!", infile);
		return 1;
	}
	if(!out) {
		printf("Output file %s could not be opened/created!", outfile);
		fclose(in);
		return 1;
	}
	
    find_labels();

    if(debugfile) {
        dbg = fopen(debugfile, "w");
        if(!dbg) {
            printf("Debug file %s could not be opened/created!", debugfile);
            return 1;
        }
        uint64_t tmp = 0xBFDE;
        fwrite(&tmp, 2, 1, dbg);        // Magic number
        fwrite(&ENTRYPOINT, 8, 1, dbg); // Program offset
        tmp = 1;
        fwrite(&tmp, 2, 1, dbg); // Only supports single files right now

        // Only supports one file:
        tmp = strlen(infile);
        fwrite(&tmp, 2, 1, dbg);     // Filename length
        fwrite(infile, tmp, 1, dbg); // Filename
        fwrite(&loc, 2, 1, dbg);     // Number of lines of code (loc)
    }

	fseek(in, 0, SEEK_SET);

	char str[1024];

	while(fgets(str, 1024, in)) {
        line++;
		// Convert entire line to uppercase
		unsigned i = 0;
		int not_white = 0, is_comment = 0, colon = 0;
		for(; i < strlen(str); i++)
		{
			str[i] = toupper(str[i]);
			if(str[i] == ':' && !is_comment) colon = 1;
			if(!not_white && str[i] == '#') is_comment = 1;
			if(!isspace(str[i])) not_white = 1;
		}

		if(colon) continue;

		if(str[0] == '#' || strlen(str) == 0 ||
			!not_white || is_comment) continue; // This line is a comment, or whitespace

		if(str[0] == '.') { // Data
            if(strstr(str, ".RESV") == str) { // Reserve words
                char *val = str + 5;
                while(isspace(*val)) val++;
                int n_words = get_number(val);
                int i = 0;
                for(; i < n_words; i++) write_word(out, 0, endian, bytes);
                continue;
            }
            if(strstr(str, ".DEFN") == str) continue;
            char *dat = get_csv_val(str + 1);
            do {
                int i = 1;
                while(isspace(dat[i])) i++;
			    if(dat[i] == '"') {
				    i++;
				    while(dat[i] != '"')
			    		write_word(out, dat[i++], endian, bytes);
			    }
			    else write_word(out, get_number(dat + i), endian, bytes);
            } while((dat = get_csv_val(NULL)) != NULL);
			continue;
		}

		char *a;
		char *b;
		char *c;

		int64_t A;
		int64_t B;
		int64_t C;

		a = strtok(str,  ", ");
		b = strtok(NULL, ", ");
		c = strtok(NULL, ", ");

        int64_t file_word = ftell(out) / bytes;
        if(dbg) { debug_write(file_word, line); }

		A = get_number(a);
		B = get_number(b);
		if(!c || c[0] == '#') C = file_word + 3 + ENTRYPOINT;
		else   C = get_number(c);

		write_op(out, A, B, C, endian, bytes);
	}

	fclose(in);
	fclose(out);

	return 0;
}


void debug_write(int64_t addr, uint32_t line) {
    fwrite(&addr, 4, 1, dbg);
    fwrite(&line, 4, 1, dbg);
}

void handle_opts(int argc, char **argv) {
    int i = 1;
    while(i < argc) {
        if(argv[i][0] == '-') {
            switch(argv[i][1]) {
                case 'h':
                    usage(0);
                    break;

                case 'i':
                    if(i == argc - 1) {
                        puts("Missing argument for -i!");
                        exit(1);
                    }
                    infile = argv[++i];
                    break;
                
                case 'o':
                    if(i == argc - 1) {
                        puts("Missing argument for -o!");
                        exit(1);
                    }
                    outfile = argv[++i];
                    break;

                case 'e':
                    if(i == argc - 1) {
                        puts("Missing argument for -e!");
                        exit(1);
                    }
                    i++;
                    if(!strcmp(argv[i], "little")) endian = 0;
                    else if(!strcmp(argv[i], "big")) endian = 1;
                    else {
                        printf("Invalid endianness: %s\n", argv[i]);
                        usage(1);
                    }
                    break;
                
                case 'b':
                    if(i == argc - 1) {
                        puts("Missing argument for -b!");
                        exit(1);
                    }
                    bytes = strtol(argv[++i], NULL, 10);
                    break;
                
                case 'E':
                    if(i == argc - 1) {
                        puts("Missing argument for -E!");
                        exit(1);
                    }
                    ENTRYPOINT = strtol(argv[++i], NULL, 0);
                    break;

                case 'd':
                    if(i == argc - 1) {
                        puts("Missing argument for -d!");
                        exit(1);
                    }
                    debugfile = argv[++i];
                    break;

                default:
                    printf("Invalid option: %s\n", argv[i]);
                    usage(1);
                    break;
            }
        } else {
            printf("Unexpected argument: %s\n", argv[i]);
            usage(1);
        }
        i++;
    }
}

void usage(int retval) {
	puts(
    "USAGE: osic-asm [OPTIONS] -i infile -o outfile\n"
	"  OPTIONS:\n"
	"    -h: Show this help message.\n"
	"    -i: Source file to assemble.\n"
	"    -o: Output binary file.\n"
    "    -e: Set endianness. Values: big, little\n"
    "    -b: Set number of bytes per integer. Values: 1-8\n"
    "    -E: Set entrypoint\n"
    "    -d: Create debugging file with specified name\n");
	exit(retval);
}
