#include <blib_macros.h>
#include <machine.h>

void m_putch(char ch) {
	*((volatile char *)PUTCHAR_ADDRESS) = ch;
}

char m_getch(void) {
	panic("please implement");
}

void m_halt(void) {
	*((volatile char *)HALT_ADDRESS) = 0;
}
