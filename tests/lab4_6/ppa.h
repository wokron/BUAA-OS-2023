#include <lib.h>

static void uassert(int cond) {
	if (!cond) {
		user_halt("OSTEST_ERR");
	}
}

static void accepted() {
	user_halt("OSTEST_OK");
}

#define tot 22
