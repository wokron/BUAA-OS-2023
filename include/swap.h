#ifndef _SWAP_H_
#define _SWAP_H_

#include <pmap.h>

// Definition of the swapable physical pages
#define SWAP_PAGE_BASE 0x3900000
#define SWAP_PAGE_END 0x3910000
#define SWAP_NPAGE ((SWAP_PAGE_END - SWAP_PAGE_BASE) / BY2PG)
#define SWAP_DISK_NPAGE 64

// Swapable Free Page List
extern struct Page_list page_free_swapable_list;
// Disk Space (Simulated)
extern u_char swap_disk[SWAP_DISK_NPAGE * BY2PG];

void swap_init();

// Functions That You Need To Implement
Pte swap_lookup(Pde *pgdir, u_int asid, u_long va);
struct Page *swap_alloc(Pde *pgdir, u_int asid);

// (Below For Recommanded Design, not necessary)
// Swapped Out Bit. 1 when 'va''s content is in disk.
#define PTE_SWP 0x0008

#endif /* _SWAP_H_ */
