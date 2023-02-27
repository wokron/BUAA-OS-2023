#include <stdio.h>

int isPalindrome(int n);

int main() {
	int n;
	scanf("%d", &n);

	if (isPalindrome(n)) {
		printf("Y\n");
	} else {
		printf("N\n");
	}
	return 0;
}

int isPalindrome(int n) {
	int num[10];
	int len = 0;
	while (n != 0) {
		num[len++] = n % 10;
		n /= 10;
	}

	for (int i = 0, j = len - 1; i < j; i++, j--) {
		if (num[i] != num[j])
			return 0;
	}
	return 1;
}
