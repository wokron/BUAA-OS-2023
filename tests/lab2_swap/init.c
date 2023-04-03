#include <swap.h>

#define ensure(expr, ...)                                                                          \
	do {                                                                                       \
		int r = ((expr) == 0);                                                             \
		if (r)                                                                             \
			panic(__VA_ARGS__);                                                        \
	} while (0)

// Index of swapable page
static inline int swap_ppn(struct Page *pp) {
	u_long pa = page2pa(pp);
	assert(SWAP_PAGE_BASE <= pa && pa < SWAP_PAGE_END);
	return PPN((pa & 0xfffff000) - SWAP_PAGE_BASE);
}

static inline void *kuseg(u_long va) {
	Pte pte = swap_lookup(cur_pgdir, 0, va);
	ensure(SWAP_PAGE_BASE <= PTE_ADDR(pte) && PTE_ADDR(pte) < SWAP_PAGE_END,
	       "Illegal Physical Page Access");
	asm volatile("mtc0 %0, $10" : : "r"(va & 0xfffff000));
	asm volatile("mtc0 %0, $2" : : "r"(pte));
	asm volatile("tlbwr" : :);
	asm volatile("nop" : :);
	return (void *)va;
}

#define TEST_VA_START 0x1000000

const char *s[] = {
    "Long ago,",
    "there was a big cat in the house.",
    "He caught many mice while they",
    " were stealing food.",
    "One day the mice had a meeting",

    " to talk about the",
    " way to deal with",
    " their common enemy.",
    "Some said this,",
    "and some said that.",

    "At last a young",
    " mouse got up,",
    "and said that he",
    " had a good idea.",
    "We could tie",

    " a bell around",
    " the neck of the cat.",
    "Then when he comes near,",
    "we can hear the",
    " sound of the bell,",

    "and run away.",
    "Everyone approved",
    " of this proposal,",
    "but an old wise",
    " mouse got up and said,",

    "That is all",
    " very well,",
    "but who will tie",
    " the bell to the cat?",
    "The mice looked",

    " at each other,",
    "but nobody spoke.",
};

static void swap_test() {
	printk("Swap Test Begin.\n");
	// Step 1: Alloc Page Directory
	struct Page *pp = NULL;
	assert(page_alloc(&pp) == 0);
	cur_pgdir = (Pde *)page2kva(pp);

	// Map All Pages
	for (int i = 0; i < SWAP_NPAGE; i++) {
		u_long va = TEST_VA_START + i * BY2PG;
		struct Page *pp = swap_alloc(cur_pgdir, 0);
		assert(pp != NULL);
		assert(page_insert(cur_pgdir, 0, pp, va, PTE_D) == 0);
		strcpy(kuseg(va), s[i]);
	}
	printk("1x Page Used\n");

	// Test ReMap Pages
	for (int i = SWAP_NPAGE; i < 2 * SWAP_NPAGE; i++) {
		u_long va = TEST_VA_START + i * BY2PG;
		struct Page *pp = swap_alloc(cur_pgdir, 0);
		assert(pp != NULL);
		assert(page_insert(cur_pgdir, 0, pp, va, PTE_D) == 0);
		strcpy(kuseg(va), s[i]);
	}

	printk("2x Page Used\n");

	for (int i = 0; i < 2 * SWAP_NPAGE; i++) {
		u_long va = TEST_VA_START + i * BY2PG;
		ensure(strcmp(kuseg(va), s[i]) == 0, "Content[%d] Wrong!", i);
	}

	printk("Congratulation!\n\n");
}

void mips_init() {
	mips_detect_memory();
	mips_vm_init();
	page_init();
	printk("Page Init Successed.\n");
	swap_init();
	printk("Swap Init Successed.\n");

	swap_test();
	halt();
}
