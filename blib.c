#include <blib.h>

size_t strlen(const char *s) {
	// panic("please implement");
	size_t len;
	for (len = 0; s[len] != '\0'; len++);
	return len;
}

char *strcpy(char *dst, const char *src) {
	// panic("please implement");
	char *s1 = dst, *s2 = src;
	while (*s1++ = *s2++);
	return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
	char *res = dst;
	while (*src && n--) {
		*dst++ = *src++;
	}
	*dst = '\0';
	return res;
}

char *strcat(char *dst, const char *src) {
	// panic("please implement");
	int len = strlen(dst);
	strcpy(dst+len, src);
	return dst;
}

int strcmp(const char *s1, const char *s2) {
	// panic("please implement");
	int len1 = strlen(s1);
	int len2 = strlen(s2);
	for (int i = 0; i < len1 || i < len2; i++) {
		int cmp = s1[i] - s2[i];
		if (0 == cmp)
			continue;
		else
			return cmp;
	}
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	while (n--) {
		if (*s1 != *s2) {
			return *s1 - *s2;
		}
		if (*s1 == 0) {
			break;
		}
		s1++;
		s2++;
	}
	return 0;
}

void *memset(void *s, int c, size_t n) {
	// panic("please implement");
	char *bytes = (char*)s;
	for (int i = 0; i < n; i++) {
		bytes[i] = (char)c;
	}
	return s;
}

void *memcpy(void *out, const void *in, size_t n) {
	char *csrc = (char *)in;
	char *cdest = (char *)out;
	for (int i = 0; i < n; i++) {
		cdest[i] = csrc[i];
	}
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	// panic("please implement");
	char *bytes1 = (char *)s1;
	char *bytes2 = (char *)s2;
	for (int i = 0; i < n; i++) {
		char cmp = bytes1[i] - bytes2[i];
		if (0 == cmp)
			continue;
		else
			return cmp;
	}
	return 0;
}
