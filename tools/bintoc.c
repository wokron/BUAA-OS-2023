#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BMAX (4 << 25)
#define FNAMEMAX 1024

static int size;
static unsigned char binary[BMAX];

static void help(void) {
	printf("convert ELF binary file to C file.\n"
	       " -h            print this message\n"
	       " -f <file>     tell the binary file  (input)\n"
	       " -o <file>     tell the c file       (output)\n"
	       " -p <prefix>   add prefix to the array name\n");
}

int main(int argc, char *args[]) {
	char *prefix = NULL;
	char *bin_file = NULL;
	char *out_file = NULL;
	int i;
	for (i = 1; i < argc; i++) {
		if (args[i][0] != '-') {
			continue;
		}
		if (strcmp(args[i], "-f") == 0) {
			assert(i + 1 < argc);
			assert(bin_file == NULL);
			bin_file = args[i + 1];
			i++;
		} else if (strcmp(args[i], "-o") == 0) {
			assert(i + 1 < argc);
			assert(out_file == NULL);
			out_file = args[i + 1];
			i++;
		} else if (strcmp(args[i], "-p") == 0) {
			assert(i + 1 < argc);
			assert(prefix == NULL);
			prefix = args[i + 1];
			i++;
		} else if (strcmp(args[i], "-h") == 0 || args[i][0] == '-') {
			help();
			return 0;
		}
	}
	assert(bin_file != NULL);
	assert(out_file != NULL);
	prefix = prefix != NULL ? prefix : "";
	FILE *bin = fopen(bin_file, "rb");
	FILE *out = fopen(out_file, "w");
	for (i = 0; bin_file[i] != '\0'; i++) {
		if (bin_file[i] == '.') {
			bin_file[i] = '\0';
			break;
		}
	}
	assert(bin != NULL);
	assert(out != NULL);
	fseek(bin, 0, SEEK_END);
	size = ftell(bin);
	fseek(bin, 0, SEEK_SET);
	assert(size < BMAX);
	size_t n = fread(binary, sizeof(char), size, bin);
	assert(n == size);
	fprintf(out,
		"unsigned int binary_%s_%s_size = %d;\n"
		"unsigned char binary_%s_%s_start[] = {",
		prefix, bin_file, size, prefix, bin_file);
	for (i = 0; i < size; i++) {
		fprintf(out, "0x%x%c", binary[i], i < size - 1 ? ',' : '}');
	}
	fputc(';', out);
	fputc('\n', out);
	fclose(bin);
	fclose(out);
	return 0;
}
