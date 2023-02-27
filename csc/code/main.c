#include <fibo.h>
#include <stdio.h>
int main() {
	int n, i;
	scanf("%d", &n);
	for (i = 1; i <= n; i++) {
		printf("%d%c", fibo(i), i == n ? '\n' : ' ');
	}
	return 0;
}
