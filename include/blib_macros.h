#ifndef _BLIB_MACROS_H__
#define _BLIB_MACROS_H__

#define STRINGIFY(s) #s
#define TOSTRING(s) STRINGIFY(s)

#define putstr(s)                                                                                  \
	({                                                                                         \
		for (const char *p = s; *p; p++)                                                   \
			m_putch(*p);                                                               \
	})

#define panic_on(cond, s)                                                                          \
	({                                                                                         \
		if (cond) {                                                                        \
			putstr("panic: ");                                                         \
			putstr(s);                                                                 \
			putstr(" @ " __FILE__ ":" TOSTRING(__LINE__) "  \n");                      \
			m_halt();                                                                  \
		}                                                                                  \
	})

#define panic(s) panic_on(1, s)

#endif
