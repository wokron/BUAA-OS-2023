#ifndef _MMU_H_
#define _MMU_H_

#include <error.h>

#define BY2PG 4096		// bytes to a page
#define PDMAP (4 * 1024 * 1024) // bytes mapped by a page directory entry
#define PGSHIFT 12
#define PDSHIFT 22 // log2(PDMAP)
#define PDX(va) ((((u_long)(va)) >> 22) & 0x03FF)
#define PTX(va) ((((u_long)(va)) >> 12) & 0x03FF)
#define PTE_ADDR(pte) ((u_long)(pte) & ~0xFFF)

// Page number field of an address
#define PPN(va) (((u_long)(va)) >> 12)
#define VPN(va) (((u_long)(va)) >> 12)

/* Page Table/Directory Entry flags */

// Global bit. When the G bit in a TLB entry is set, that TLB entry will match solely on the VPN
// field, regardless of whether the TLB entry’s ASID field matches the value in EntryHi.
#define PTE_G 0x0100

// Valid bit. If 0 any address matching this entry will cause a tlb miss exception (TLBL/TLBS).
#define PTE_V 0x0200

// Dirty bit, but really a write-enable bit. 1 to allow writes, 0 and any store using this
// translation will cause a tlb mod exception (TLB Mod).
#define PTE_D 0x0400

// Uncacheable bit. 0 to make the access cacheable, 1 for uncacheable.
#define PTE_N 0x0800

// Copy On Write. Reserved for software, used by fork.
#define PTE_COW 0x0001

// Shared memmory. Reserved for software, used by fork.
#define PTE_LIBRARY 0x0004

// Memory segments (32-bit kernel mode addresses)
#define KUSEG 0x00000000U
#define KSEG0 0x80000000U
#define KSEG1 0xA0000000U
#define KSEG2 0xC0000000U

/*
 * Part 2.  Our conventions.
 */

/*
 o     4G ----------->  +----------------------------+------------0x100000000
 o                      |       ...                  |  kseg2
 o      KSEG2    -----> +----------------------------+------------0xc000 0000
 o                      |          Devices           |  kseg1
 o      KSEG1    -----> +----------------------------+------------0xa000 0000
 o                      |      Invalid Memory        |   /|\
 o                      +----------------------------+----|-------Physical Memory Max
 o                      |       ...                  |  kseg0
 o      KSTACKTOP-----> +----------------------------+----|-------0x8040 0000-------end
 o                      |       Kernel Stack         |    | KSTKSIZE            /|\
 o                      +----------------------------+----|------                |
 o                      |       Kernel Text          |    |                    PDMAP
 o      KERNBASE -----> +----------------------------+----|-------0x8001 0000    |
 o                      |      Exception Entry       |   \|/                    \|/
 o      ULIM     -----> +----------------------------+------------0x8000 0000-------
 o                      |         User VPT           |     PDMAP                /|\
 o      UVPT     -----> +----------------------------+------------0x7fc0 0000    |
 o                      |           pages            |     PDMAP                 |
 o      UPAGES   -----> +----------------------------+------------0x7f80 0000    |
 o                      |           envs             |     PDMAP                 |
 o  UTOP,UENVS   -----> +----------------------------+------------0x7f40 0000    |
 o  UXSTACKTOP -/       |     user exception stack   |     BY2PG                 |
 o                      +----------------------------+------------0x7f3f f000    |
 o                      |                            |     BY2PG                 |
 o      USTACKTOP ----> +----------------------------+------------0x7f3f e000    |
 o                      |     normal user stack      |     BY2PG                 |
 o                      +----------------------------+------------0x7f3f d000    |
 a                      |                            |                           |
 a                      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                           |
 a                      .                            .                           |
 a                      .                            .                         kuseg
 a                      .                            .                           |
 a                      |~~~~~~~~~~~~~~~~~~~~~~~~~~~~|                           |
 a                      |                            |                           |
 o       UTEXT   -----> +----------------------------+------------0x0040 0000    |
 o                      |      reserved for COW      |     BY2PG                 |
 o       UCOW    -----> +----------------------------+------------0x003f f000    |
 o                      |   reversed for temporary   |     BY2PG                 |
 o       UTEMP   -----> +----------------------------+------------0x003f e000    |
 o                      |       invalid memory       |                          \|/
 a     0 ------------>  +----------------------------+ ----------------------------
 o
*/

#define KERNBASE 0x80010000

#define KSTACKTOP (ULIM + PDMAP)
#define ULIM 0x80000000

#define UVPT (ULIM - PDMAP)
#define UPAGES (UVPT - PDMAP)
#define UENVS (UPAGES - PDMAP)

#define UTOP UENVS
#define UXSTACKTOP UTOP

#define USTACKTOP (UTOP - 2 * BY2PG)
#define UTEXT PDMAP
#define UCOW (UTEXT - BY2PG)
#define UTEMP (UCOW - BY2PG)

#ifndef __ASSEMBLER__

/*
 * Part 3.  Our helper functions.
 */
#include <string.h>
#include <types.h>

extern u_long npage;

typedef u_long Pde;
typedef u_long Pte;

#define PADDR(kva)                                                                                 \
	({                                                                                         \
		u_long a = (u_long)(kva);                                                          \
		if (a < ULIM)                                                                      \
			panic("PADDR called with invalid kva %08lx", a);                           \
		a - ULIM;                                                                          \
	})

// translates from physical address to kernel virtual address
#define KADDR(pa)                                                                                  \
	({                                                                                         \
		u_long ppn = PPN(pa);                                                              \
		if (ppn >= npage) {                                                                \
			panic("KADDR called with invalid pa %08lx", (u_long)pa);                   \
		}                                                                                  \
		(pa) + ULIM;                                                                       \
	})

#define assert(x)                                                                                  \
	do {                                                                                       \
		if (!(x)) {                                                                        \
			panic("assertion failed: %s", #x);                                         \
		}                                                                                  \
	} while (0)

#define TRUP(_p)                                                                                   \
	({                                                                                         \
		typeof((_p)) __m_p = (_p);                                                         \
		(u_int) __m_p > ULIM ? (typeof(_p))ULIM : __m_p;                                   \
	})

extern void tlb_out(u_int entryhi);
#endif //!__ASSEMBLER__
#endif // !_MMU_H_
