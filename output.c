#include <drivers/dev_cons.h>
#include <stddef.h>

void printcharc(char ch) {
	*((volatile char *)(0xA0000000 + DEV_CONS_ADDRESS + DEV_CONS_PUTGETCHAR)) = ch;
}

void halt(void) {
	*((volatile char *)(0xA0000000 + DEV_CONS_ADDRESS + DEV_CONS_HALT)) = 0;
}

void print_str(const char *buf) {
	for (int i = 0; buf[i]; i++) {
		printcharc(buf[i]);
	}
}

void print_num(unsigned long u) {
	if (u < 0) {
		printcharc('-');
		u = -u;
	}
	char digit = '0' + u % 10;
	u /= 10;
	if (u != 0) {
		print_num(u);
	}
	printcharc(digit);
}
