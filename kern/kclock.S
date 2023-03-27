#include <asm/asm.h>
#include <drivers/dev_rtc.h>
#include <kclock.h>
#include <mmu.h>

LEAF(kclock_init)
	li      t0, 200 // the timer interrupt frequency in Hz

	/* Write 't0' into the timer (RTC) frequency register.
	 *
	 * Hint:
	 *   You may want to use 'sw' instruction and constants 'DEV_RTC_ADDRESS' and
	 *   'DEV_RTC_HZ' defined in include/drivers/dev_rtc.h.
	 *   To access device through mmio, a physical address must be converted to a
	 *   kseg1 address.
	 *
	 * Reference: http://gavare.se/gxemul/gxemul-stable/doc/experiments.html#expdevices
	 */
	/* Exercise 3.11: Your code here. */
	la	t1, KSEG1
	la	t2, DEV_RTC_ADDRESS
	la	t3, DEV_RTC_HZ
	or	t1, t1, t2
	or	t1, t1, t3
	sw	t0, 0(t1)

	jr      ra
END(kclock_init)
