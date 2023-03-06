#include <stdio.h>
#include <stdlib.h>

extern int readelf(void *binary, size_t size);

/*
	Open the ELF file specified in the argument, and call the function 'readelf'
	to parse it.
	Params:
		argc: the number of parameters
		argv: array of parameters, argv[1] should be the file name.
*/
int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <elf-file>\n", argv[0]);
		return 1;
	}

	FILE *fp = fopen(argv[1], "rb");
	if (fp == NULL) {
		perror(argv[1]);
		return 1;
	}

	if (fseek(fp, 0, SEEK_END)) {
		perror("fseek");
		goto err;
	}
	int fsize = ftell(fp);
	if (fsize < 0) {
		perror("ftell");
		goto err;
	}

	char *p = malloc(fsize + 1);
	if (p == NULL) {
		perror("malloc");
		goto err;
	}
	if (fseek(fp, 0, SEEK_SET)) {
		perror("fseek");
		goto err;
	}
	if (fread(p, fsize, 1, fp) < 0) {
		perror("fread");
		goto err;
	}
	p[fsize] = 0;

	return readelf(p, fsize);
err:
	fclose(fp);
	return 1;
}
