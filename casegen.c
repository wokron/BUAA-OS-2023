#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *ops[] = {"add", "sub", "mul", "div"};

uint32_t xrand() {
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	static uint32_t x = 2023;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return x;
}

void help() {
	printf("Usage: ./casegen <op> <num>\n"
	       "Available <op>s: add, sub, mul, div.\n"
	       "<num>: The number of test cases generated (a positive integer).\n");
}

int is_op_legal(char *op) {
	size_t sz = sizeof(ops) / sizeof(ops[0]);
	for (int i = 0; i < sz; i++) {
		if (strcmp(op, ops[i]) == 0) {
			return 1;
		}
	}
	return 0;
}

int get_num(char *str_num, int *num) {
	*num = strtol(str_num, NULL, 10);
	return *num > 0;
}

int main(int argc, char **argv) {
	int num;
	if (argc != 3 || !is_op_legal(argv[1]) || !get_num(argv[2], &num)) {
		help();
		return 1;
	}
	while (num--) {
		printf("%s %u %u\n", argv[1], xrand() % 10000 + 1, xrand() % 10000 + 1);
	}
	return 0;
}
