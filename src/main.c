#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include <write.h>

uint64_t ENTRYPOINT = 0;
int endian = 0;
int bytes = 2;
int loc = 0; // Lines of code

static char *infile    = NULL;
static char *outfile   = NULL;
static char *debugfile = NULL;

static FILE *in  = NULL;
static FILE *out = NULL;
static FILE *dbg = NULL;

static int max_labels = 1024; // Maximum number of lables allowed

struct label
{
	char     name[32];
	uint32_t hash;
	int16_t addr;
	uint32_t line;
};

struct label *labels;

void handle_opts(int, char **);
void usage(int);
int64_t get_number(char *);
void find_labels();
void debug_write(int64_t, uint32_t);
char *get_csv_val(char *str);

int main(int argc, char **argv)
{
	handle_opts(argc, argv);

	if(!infile || !outfile) usage(1);

	in  = fopen(infile, "r");
	out = fopen(outfile, "w");

	if(!in)
	{
		printf("Input file %s could not be opened!", infile);
		return 1;
	}
	if(!out)
	{
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
    uint32_t line = 0;

	while(fgets(str, 1024, in))
	{
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

		if(str[0] == '.') // Data
		{
            char *dat = get_csv_val(str + 1);
            do {
                int i = 1;
                while(dat[i] == ' ') i++;
			    if(dat[i] == '"') // There MUST be a space inbewteen
			    {
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


uint32_t strhash(char *str)
{
	uint32_t hash = 0;
	while(*str)
	{
		uint8_t end = ((hash >> 24) & 0xFF);
		hash = (hash << 8) | (*str ^ end);
		str++;
	}
	return hash;
}

void find_labels()
{
	labels = malloc(sizeof(struct label) * max_labels);

	uint32_t line = 1;
	int16_t addr = ENTRYPOINT; // Place it above the device-mapped memory
	int c_idx = 0;

	char str[1024];

	while(fgets(str, 1024, in))
	{
		// Convert entire line to uppercase
		unsigned i = 0;
		int not_white = 0, is_comment = 0, colon = 0;
		for(; i < strlen(str); i++)
		{
			str[i] = toupper(str[i]);
			if(str[i] == ':' && !is_comment) colon = i; // If it is the first char on the line, ignore it
			if(!not_white && str[i] == '#') is_comment = 1;
			if(!isspace(str[i])) not_white = 1;
		}

		if(colon) // We found a label
		{
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

		if(str[0] == '.') // Data
		{
            char *dat = get_csv_val(str + 1);
            do {
                int i = 1;
                while(dat[i] == ' ') i++;
			    if(dat[i] == '"') // There MUST be a space inbewteen
			    {
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

	for(; i < max_labels; i++)
	{
		if(labels[i].hash == hash)
			if(!strcmp(labels[i].name, name)) { // Just incase two labels have the same hash
				return i;
            }
	}
	return -1; // Not found...
}


int64_t get_number(char *str)
{
	while(isspace(*str)) str++;
    if(!isdigit(*str) && (*str != '+' && *str != '-'))
	{
		uint64_t i = 0;
		int64_t off = 0; // Signed, because it can be a negative offset

		for(; i < strlen(str); i++)
		{
			if(str[i] == '$') // Used to represent an array index
			{
				str[i++] = '\0';
				off = get_number(&str[i]);
			}
            if(str[i] == '\n' || str[i] == ' ') {
                str[i] = '\0';
                break;
            }
		}
		int idx = find_label(str);
		if(idx == -1)
		{
			printf("ERROR: Could not find label: '%s'", str);
			exit(1);
		}
		return labels[idx].addr + off;
	}

	return strtoll(str, NULL, 0);
}

void debug_write(int64_t addr, uint32_t line) {
    fwrite(&addr, 4, 1, dbg);
    fwrite(&line, 4, 1, dbg);
}

void handle_opts(int argc, char **argv)
{
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

void usage(int retval)
{
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
