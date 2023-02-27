#include <fibo.h>
char fibo(char n) {
	char a;
	char b;
	char c;
	if (n == 1) {
		a = 1;
	} else {
		char i;
		c = 0;
		b = 1;
		for (i = 2; i <= n; i++) {
			a = b + c;
			c = b;
			b = a;
		}
	}
	return a;
}
