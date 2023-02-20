#include "output.h"
#include <stddef.h>

void hello(int a, int b, int c, int d, int e, int f) {
	print_str("hello world\n");
	print_num(a);
	print_str("\n");
	print_num(b);
	print_str("\n");
	print_num(c);
	print_str("\n");
	print_num(d);
	print_str("\n");
	print_num(e);
	print_str("\n");
	print_num(f);
	print_str("\n");
	halt();
}
