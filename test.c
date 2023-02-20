#include <blib.h>

const char *s[] = {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
	     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab",
	     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
	     ", World!\n",
	     "Hello, World!\n",
	     "#####"};

char str1[] = "Hello";
char str[20];

int main() {
	putstr("begin check m_getch()\n");
	char ch;
	while ((ch = m_getch()) == 0) {
		;
	}
	putstr("get the char ! ch=");
	m_putch(ch);
	m_putch('\n');
	putstr("begin check string\n");
	panic_on((strncmp(s[0], s[1], 100) >= 0), "check failed at");
	panic_on((strncmp(s[1], s[0], 100) <= 0), "check failed at");
	panic_on((strncmp(s[0], s[1], 30) != 0), "check failed at");
	panic_on((strncmp("aa", s[0], 2) != 0), "check failed at");
	panic_on((strncmp("aa", s[0], 100) >= 0), "check failed at");
	panic_on((strncmp(s[0], "aa", 100) <= 0), "check failed at");

	size_t n = strlen(s[0]);
	panic_on((strlen(s[0]) != 38), "check failed at");
	panic_on((strlen("") != 0), "check failed at");

	panic_on((strcmp(s[0], s[2]) != 0), "check failed at");
	panic_on((strcmp(s[0], s[1]) >= 0), "check failed at");
	panic_on((strcmp(s[0] + 1, s[1] + 1) >= 0), "check failed at");
	panic_on((strcmp(s[0] + 2, s[1] + 2) >= 0), "check failed at");
	panic_on((strcmp(s[0] + 3, s[1] + 3) >= 0), "check failed at");
	panic_on((strcmp(s[1], s[0]) <= 0), "check failed at");
	panic_on((strcmp("aa", s[1]) >= 0), "check failed at");
	panic_on((strcmp(s[1], "aa") <= 0), "check failed at");

	char *tmp = strncpy(str, str1, 3);
	panic_on((strncmp(tmp, str1, 3) != 0), "check failed at");
	tmp = strcpy(str, str1);
	panic_on((strcmp(tmp, str1) != 0), "check failed at");
	tmp = strcat(tmp, s[3]);
	panic_on((strcmp(tmp, s[4]) != 0), "check failed at");

	panic_on((memcmp(s[0], s[2], n) != 0), "check failed at");
	panic_on((memcmp(s[0], s[1], n) >= 0), "check failed at");
	panic_on((memcmp(s[1], s[0], n) <= 0), "check failed at");

	void *mem = memset(str, '#', 5);
	panic_on((memcmp(mem, s[5], 5) != 0), "check failed at");
	putstr("string test pass!\n");

	return 0;
}
