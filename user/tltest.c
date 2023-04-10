#include <lib.h>

int main() {
	debugf("Smashing some kernel codes...\n"
	       "If your implementation is correct, you may see unknown exception here:\n");
	*(int *)KERNBASE = 0;
	debugf("My mission completed!\n");
	return 0;
}
