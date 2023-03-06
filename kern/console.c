#include <drivers/dev_cons.h>
#include <mmu.h>

void printcharc(char ch) {
	*((volatile char *)(KSEG1 + DEV_CONS_ADDRESS + DEV_CONS_PUTGETCHAR)) = ch;
}

char scancharc(void) {
	return *((volatile char *)(KSEG1 + DEV_CONS_ADDRESS + DEV_CONS_PUTGETCHAR));
}

void halt(void) {
	*((volatile char *)(KSEG1 + DEV_CONS_ADDRESS + DEV_CONS_HALT)) = 0;
}
