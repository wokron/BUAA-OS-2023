#include <stdio.h>
#include <string.h>

int main() {
	char op[5];
	int a, b;
	while (scanf("%s%d%d", op, &a, &b) == 3) {
		if (!strcmp(op, "add")) {
			printf("%d\n", a + b);
		} else if (!strcmp(op, "sub")) {
			printf("%d\n", a - b);
		} else if (!strcmp(op, "mul")) {
			printf("%d\n", a * b);
		} else if (!strcmp(op, "div")) {
			printf("%d\n", a / b);
		}
	}
	return 0;
}
