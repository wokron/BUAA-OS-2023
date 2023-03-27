#include <stdarg.h>

int nums[520] = {
    925,   2422, 1794, 3147, 673,  3169, 2858, 2865, 3070, 372,	 2437, 4005, 2260, 1832, 2555, 1617,
    2194,  3729, 595,  1196, 2913, 2277, 1025, 1356, 3045, 2763, 2533, 1198, 3227, 2066, 2924, 1545,
    676,   1145, 1394, 710,  418,  3181, 964,  320,  3427, 298,	 2436, 2302, 2904, 1308, 2560, 1773,
    1972,  4145, 2634, 2509, 479,  3586, 2301, 3250, 1616, 2755, 3938, 994,  1881, 302,	 1132, 2094,
    2352,  1510, 3378, 1080, 2733, 2563, 1430, 1804, 1603, 2981, 817,  987,  1062, 1239, 2193, 2206,
    3569,  1476, 3759, 1943, 2299, 1194, 1416, 2344, 2558, 2536, 1153, 2323, 3289, 2734, 2830, 2759,
    3740,  1208, 1873, 653,  2323, 2657, 2832, 3789, 1548, 2748, 2841, 2602, 1509, -5,	 3077, -518,
    1765,  991,	 1231, 1302, 1912, 2612, 2943, 1082, 1288, 3435, 2039, 3200, 2140, 4620, 2411, 1118,
    2943,  2116, 2756, 957,  3278, 2658, 4707, 1622, 2951, 1525, 1564, 2974, 2506, 2335, 1512, 52,
    2195,  1244, 451,  349,  2538, 2313, 3316, 1323, 2123, -489, 2716, 2496, 2547, 1100, 1732, 2825,
    1064,  2547, 1969, 2553, 3927, 2379, 2017, 632,  2171, 2165, 1713, 3551, 785,  2363, 1687, 2137,
    1023,  2976, 621,  1762, 1930, 1205, 919,  1422, 1651, 1706, 1902, 1966, 1764, 3420, 1576, 679,
    1302,  1989, 2640, 2731, 2405, 1006, 2214, 2260, 968,  3012, 920,  1862, 3447, 1762, 2219, 1556,
    1595,  485,	 1081, 2434, 3153, 863,	 1972, 1367, 1746, 3754, 4136, 3789, 1687, 1511, 3014, 801,
    1634,  -949, 1116, 1748, 4377, 3738, 2284, 1278, 1924, 2507, 2295, 2428, 1763, 1768, 1531, 2747,
    2781,  2530, 3849, 1371, 1693, 2104, 2750, 1643, 2371, 964,	 1849, 3274, 1521, 4170, 194,  2072,
    2334,  2200, 3583, 836,  2350, 3207, 633,  2538, 1096, 2389, 1530, 1490, 3227, 260,	 1750, 2280,
    3425,  1858, 2766, 1239, 388,  1515, 1236, 1096, 1190, 737,	 878,  4488, 1408, 2528, 3536, 1733,
    2099,  3753, 216,  3469, 2284, 1082, 2269, 1098, 1092, 1036, 14,   1433, 2203, 1680, 2287, 1929,
    -1017, 132,	 3490, 2829, 1790, 1948, 1557, 2690, 1608, 1772, 1614, 653,  1813, 2591, 2731, 3751,
    388,   1444, 296,  2587, -257, 2582, 1178, 3884, 4067, 1770, 1815, 3301, 1742, 2153, 2463, 2304,
    2895,  3866, 2290, 2827, 2333, 1896, 1986, 2735, 4420, 3533, 2879, 1503, 2110, 4387, 1972, 2080,
    3425,  2451, 2380, 1004, 1889, 1404, 964,  2788, 1057, 667,	 2128, 1800, 729,  475,	 1201, 748,
    779,   -11,	 2342, 1362, 2732, 3439, 1727, 2681, 1249, 2407, 2373, 1787, 2788, 1802, 2382, 1960,
    1249,  3027, 2311, 2684, 1327, 3458, 2363, 1683, 3359, 2920, 1192, -544, 2265, 2028, 1200, 493,
    3115,  3238, 665,  2072, 3323, 2014, 1794, 1945, 1302, 1898, 3429, 1502, 1660, -523, 1532, 2985,
    2182,  2520, 2681, 2419, 2161, 1589, 2405, 1569, 2861, 2168, 1215, 2438, 3396, 3336, 2949, 1982,
    980,   2794, 3045, 675,  1613, 2034, 2701, 1823, 3599, 1426, 1768, 1256, 1525, 2547, -13,  3019,
    3394,  1136, 905,  834,  3745, 1345, 1466, 2406, 1552, 3363, 2340, 1107, 2593, 1551, -44,  1614,
    4791,  2259, 3452, 1278, 2240, 1403, 2461, 1735, 502,  -19,	 3284, 535,  1710, 1064, 1986, 3304,
    906,   3194, 1057, 2402, 1722, 1442, 1498, 1605, 2020, 849,	 4648, 3,    1620, 399,	 2139, 735,
    2567,  902,	 2105, 2617};

int partition(int arr[], int low, int high) {
	int key;
	key = arr[low];
	while (low < high) {
		while (low < high && arr[high] >= key) {
			high--;
		}
		if (low < high) {
			arr[low++] = arr[high];
		}
		while (low < high && arr[low] <= key) {
			low++;
		}
		if (low < high) {
			arr[high--] = arr[low];
		}
	}
	arr[low] = key;
	return low;
}

void quick_sort(int arr[], int start, int end) {
	int pos;
	if (start < end) {
		pos = partition(arr, start, end);
		quick_sort(arr, start, pos - 1);
		quick_sort(arr, pos + 1, end);
	}
}

#define LP_MAX_BUF 1000
unsigned int hash = 1;

static int PrintChar(char *, char, int, int);
static int PrintString(char *, const char *, int, int);
static int PrintNum(char *, unsigned long, int, int, int, int, char, int);

static void print_buf(const char *s, int l) {
	int i;
	for (i = 0; i < l; i++) {
		hash = hash * 131 + s[i] + 1;
	}
}

void vprintf(const char *fmt, va_list ap) {

	static const char theFatalMsg[] = "fatal error in vprintf\n";

#define OUTPUT(s, l)                                                                               \
	do {                                                                                       \
		if (((l) < 0) || ((l) > LP_MAX_BUF)) {                                             \
			print_buf(theFatalMsg, sizeof(theFatalMsg) - 1);                           \
			for (;;)                                                                   \
				;                                                                  \
		} else {                                                                           \
			print_buf(s, l);                                                           \
		}                                                                                  \
	} while (0)

	char buf[LP_MAX_BUF];

	char c;
	const char *s;
	long int num;

	int longFlag;
	int negFlag;
	int width;
	int prec;
	int ladjust;
	char padc;
	int length;

	for (;;) {
		length = 0;
		s = fmt;
		for (; *fmt != '\0'; fmt++) {
			if (*fmt != '%') {
				length++;
			} else {
				OUTPUT(s, length);
				length = 0;
				fmt++;
				break;
			}
		}
		OUTPUT(s, length);
		if (!*fmt) {
			break;
		}
		ladjust = 0;
		padc = ' ';
		if (*fmt == '-') {
			ladjust = 1;
			padc = ' ';
			fmt++;
		} else if (*fmt == '0') {
			ladjust = 0;
			padc = '0';
			fmt++;
		}
		width = 0;
		while ((*fmt >= '0') && (*fmt <= '9')) {
			width = width * 10 + (*fmt) - '0';
			fmt++;
		}
		prec = 0;
		if (*fmt == '.') {
			fmt++;
			while ((*fmt >= '0') && (*fmt <= '9')) {
				prec = prec * 10 + (*fmt) - '0';
				fmt++;
			}
		}
		longFlag = 0;
		while (*fmt == 'l') {
			longFlag = 1;
			fmt++;
		}
		negFlag = 0;
		switch (*fmt) {
		case 'b':
			if (longFlag) {
				num = va_arg(ap, long int);
			} else {
				num = va_arg(ap, int);
			}
			length = PrintNum(buf, num, 2, 0, width, ladjust, padc, 0);
			OUTPUT(buf, length);
			break;
		case 'd':
		case 'D':
			if (longFlag) {
				num = va_arg(ap, long int);
			} else {
				num = va_arg(ap, int);
			}
			negFlag = num < 0;
			num = negFlag ? -num : num;
			length = PrintNum(buf, num, 10, negFlag, width, ladjust, padc, 0);
			OUTPUT(buf, length);
			break;
		case 'o':
		case 'O':
			if (longFlag) {
				num = va_arg(ap, long int);
			} else {
				num = va_arg(ap, int);
			}
			length = PrintNum(buf, num, 8, 0, width, ladjust, padc, 0);
			OUTPUT(buf, length);
			break;
		case 'u':
		case 'U':
			if (longFlag) {
				num = va_arg(ap, long int);
			} else {
				num = va_arg(ap, int);
			}
			length = PrintNum(buf, num, 10, 0, width, ladjust, padc, 0);
			OUTPUT(buf, length);
			break;
		case 'x':
			if (longFlag) {
				num = va_arg(ap, long int);
			} else {
				num = va_arg(ap, int);
			}
			length = PrintNum(buf, num, 16, 0, width, ladjust, padc, 0);
			OUTPUT(buf, length);
			break;
		case 'X':
			if (longFlag) {
				num = va_arg(ap, long int);
			} else {
				num = va_arg(ap, int);
			}
			length = PrintNum(buf, num, 16, 0, width, ladjust, padc, 1);
			OUTPUT(buf, length);
			break;
		case 'c':
			c = (char)va_arg(ap, int);
			length = PrintChar(buf, c, width, ladjust);
			OUTPUT(buf, length);
			break;
		case 's':
			s = (char *)va_arg(ap, char *);
			length = PrintString(buf, s, width, ladjust);
			OUTPUT(buf, length);
			break;
		case '\0':
			fmt--;
			break;
		default:
			OUTPUT(fmt, 1);
		}
		fmt++;
	}
}

int PrintChar(char *buf, char c, int length, int ladjust) {
	int i;

	if (length < 1) {
		length = 1;
	}
	if (ladjust) {
		*buf = c;
		for (i = 1; i < length; i++) {
			buf[i] = ' ';
		}
	} else {
		for (i = 0; i < length - 1; i++) {
			buf[i] = ' ';
		}
		buf[length - 1] = c;
	}
	return length;
}

int PrintString(char *buf, const char *s, int length, int ladjust) {
	int i;
	int len = 0;
	const char *s1 = s;
	while (*s1++) {
		len++;
	}
	if (length < len) {
		length = len;
	}

	if (ladjust) {
		for (i = 0; i < len; i++) {
			buf[i] = s[i];
		}
		for (i = len; i < length; i++) {
			buf[i] = ' ';
		}
	} else {
		for (i = 0; i < length - len; i++) {
			buf[i] = ' ';
		}
		for (i = length - len; i < length; i++) {
			buf[i] = s[i - length + len];
		}
	}
	return length;
}

int PrintNum(char *buf, unsigned long u, int base, int negFlag, int length, int ladjust, char padc,
	     int upcase) {
	int actualLength = 0;
	char *p = buf;
	int i;
	do {
		int tmp = u % base;
		if (tmp <= 9) {
			*p++ = '0' + tmp;
		} else if (upcase) {
			*p++ = 'A' + tmp - 10;
		} else {
			*p++ = 'a' + tmp - 10;
		}
		u /= base;
	} while (u != 0);

	if (negFlag) {
		*p++ = '-';
	}

	actualLength = p - buf;
	if (length < actualLength) {
		length = actualLength;
	}

	if (ladjust) {
		padc = ' ';
	}
	if (negFlag && !ladjust && (padc == '0')) {
		for (i = actualLength - 1; i < length - 1; i++) {
			buf[i] = padc;
		}
		buf[length - 1] = '-';
	} else {
		for (i = actualLength; i < length; i++) {
			buf[i] = padc;
		}
	}

	int begin = 0;
	int end;
	if (ladjust) {
		end = actualLength - 1;
	} else {
		end = length - 1;
	}

	while (end > begin) {
		char tmp = buf[begin];
		buf[begin] = buf[end];
		buf[end] = tmp;
		begin++;
		end--;
	}
	return length;
}

void printf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

int main() {
	int i;
	static int arr[20] = {32, 12, 7, 78, 23, 45};

	arr[6] = 666;
	arr[7] = 999;

	for (i = 0; i < 8; ++i) {
		nums[i] = arr[7 - i];
	}

	quick_sort(nums, 0, 520 - 1);

	for (i = 0; i < 100; i++) {
		printf("%d ", nums[i]);
		if ((i + 1) % 10 == 0) {
			printf("\n");
		}
	}

	for (i = 101; i < 520; i += 11) {
		printf("%d ", nums[i]);
		if ((i - 100) % 10 == 0) {
			printf("\n");
		}
	}
	return hash;
}
