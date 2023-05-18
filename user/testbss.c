#include <lib.h>
#define ARRAYSIZE 0x1000000
int bigarray[ARRAYSIZE] = {0};
int main(int argc, char **argv) {
	int i;

	debugf("Making sure bss works right... \n");
	for (i = 0; i < ARRAYSIZE; i++) {
		if (bigarray[i] != 0) {
			user_panic("bigarray[%d] isn't cleared!\n", i);
		}
	}
	for (i = 0; i < ARRAYSIZE; i++) {
		bigarray[i] = i;
	}
	for (i = 0; i < ARRAYSIZE; i++) {
		if (bigarray[i] != i) {
			user_panic("bigarray[%d] didn't hold its value!\n", i);
		}
	}
	debugf("Bss is good\n");
	return 0;
}
