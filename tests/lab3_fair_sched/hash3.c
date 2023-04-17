#define MOD 99999993

int strlen(char *s) {
	int i;
	for (i = 0; s[i]; i++) {
		;
	}
	return i;
}

int main() {
	int i;
	char str[] = "Hello World";
	int len = strlen(str);
	int hash = 0;
	for (i = 0; i < 12000000; i++) {
		hash = hash * 10 + str[i % len];
		hash %= MOD;
	}
	return hash;
}
