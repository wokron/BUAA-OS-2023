int a = 65534;
int b = 15269;

typedef struct _person {
	int time;
	char name[20];
} Person;

Person mike;
Person jack;

int talk[1000];
int names[1000] = {'A'};

char func(char x) {
	static char m[20] __attribute__((unused)) = {2};
	return x - 5;
}

int my_cal(int a, int b) {
	return a + b;
}

void *memset(void *dest, int c, unsigned int n) {
	for (unsigned int i = 0; i < n; i++) {
		((char *)dest)[i] = c;
	}
	return dest;
}

int main() {
	int array[1000] __attribute__((unused)) = {1};
	int x __attribute__((unused)) = my_cal(5, 6);
	char c __attribute__((unused)) = func('Z');
	while (1) {
	}
	return 0;
}
