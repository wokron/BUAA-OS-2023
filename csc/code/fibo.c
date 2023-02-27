#include <fibo.h>
int fibo(int n) {
	int a;
	int b;
	int c;
	if (n == 1) {
		a = 1;
	} else {
		int i;
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
