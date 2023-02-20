#ifndef _MACHINE_H
#define _MACHINE_H
/* gxemul device physical address
  for more information about devices please access
  http://gavare.se/gxemul/gxemul-stable/doc/experiments.html#expdevices
*/
#define DEV_RAM_PADDR_LOW 0x00000000
#define DEV_RAM_PADDR_HIGH 0x03FFFFFF /* we have 64 MB RAM */

#define DEV_CONS_PADDR 0X10000000 /* console base paddr */
#define DEV_CONS_PUTGETCHAR_OFFSET 0x00000000
#define DEV_CONS_HALT_OFFSET 0x00000010

/* always use kseg1 to access console and halt.
  think why ?
 */
#define PUTCHAR_ADDRESS (KSEG1_VADDR_LOW + DEV_CONS_PADDR + DEV_CONS_PUTGETCHAR_OFFSET)
#define GETCHAR_ADDRESS (KSEG1_VADDR_LOW + DEV_CONS_PADDR + DEV_CONS_PUTGETCHAR_OFFSET)
#define HALT_ADDRESS (KSEG1_VADDR_LOW + DEV_CONS_PADDR + DEV_CONS_HALT_OFFSET)

/* MIPS architecture virtual address space define.
  for more information please access idtR3000.pdf
*/
#define KSEG1_VADDR_HIGH 0xbfffffff
#define KSEG1_VADDR_LOW 0xa0000000
#define KSEG0_VADDR_HIGH 0x9fffffff
#define KSEG0_VADDR_LOW 0x80000000
#define KUSEG_VADDR_HIGH 0x7fffffff
#define KUSEG_VADDR_LOW 0x00000000

/* IO function */
void m_putch(char ch);
char m_getch(void);
/* when call this func, gxemul will quit */
void m_halt(void);

#endif
