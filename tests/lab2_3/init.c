void tlb_refill_check(void) {
	Pde *boot_pgdir = alloc(BY2PG, BY2PG, 1);
	cur_pgdir = boot_pgdir;
	struct Page *pp, *pp0, *pp1, *pp2, *pp3, *pp4;

	// should be able to allocate three pages
	pp0 = pp1 = pp2 = pp3 = pp4 = 0;
	assert(page_alloc(&pp0) == 0);
	assert(page_alloc(&pp1) == 0);
	assert(page_alloc(&pp2) == 0);
	assert(page_alloc(&pp3) == 0);
	assert(page_alloc(&pp4) == 0);

	// temporarily steal the rest of the free pages
	// now this page_free list must be empty!!!!
	LIST_INIT(&page_free_list);

	// free pp0 and try again: pp0 should be used for page table
	page_free(pp0);
	// check if PTE != PP
	assert(page_insert(boot_pgdir, 0, pp1, 0x0, 0) == 0);
	// should be able to map pp2 at BY2PG because pp0 is already allocated for page table
	assert(page_insert(boot_pgdir, 0, pp2, BY2PG, 0) == 0);

	printk("tlb_refill_check() begin!\n");

	extern void do_tlb_refill(void);
	extern Pte _do_tlb_refill(u_long va, u_int asid);

	Pte *walk_pte;
	Pte ret_pte = _do_tlb_refill(BY2PG, 0);
	assert(page_lookup(boot_pgdir, BY2PG, &walk_pte) != NULL);
	assert(ret_pte == *walk_pte);
	assert(page2pa(pp2) == va2pa(boot_pgdir, BY2PG));

	printk("test point 1 ok\n");

	page_free(pp4);
	page_free(pp3);

	assert(page_lookup(boot_pgdir, 0x00400000, &walk_pte) == NULL);
	ret_pte = _do_tlb_refill(0x00400000, 0);
	assert((pp = page_lookup(boot_pgdir, 0x00400000, &walk_pte)) != NULL);
	assert(va2pa(boot_pgdir, 0x00400000) == page2pa(pp3));

	printk("test point 2 ok\n");

	u_long badva, entryhi, entrylo, index;
	badva = 0x00400000;
	entryhi = badva & 0xfffff000;
	asm volatile("mtc0 %0, $8" : : "r"(badva));
	asm volatile("mtc0 %0, $10" : : "r"(entryhi));
	do_tlb_refill();

	entrylo = 0;
	index = -1;
	badva = 0x00400000;
	entryhi = badva & 0xfffff000;
	asm volatile("mtc0 %0, $10" : : "r"(entryhi));
	asm volatile("mtc0 %0, $2" : : "r"(entrylo));
	asm volatile("mtc0 %0, $0" : : "r"(index));
	asm volatile("tlbp" : :);
	asm volatile("nop" : :);

	asm volatile("mfc0 %0, $0" : "=r"(index) :);
	assert(index >= 0);
	asm volatile("tlbr" : :);
	asm volatile("mfc0 %0, $2" : "=r"(entrylo) :);
	assert(entrylo == ret_pte);

	printk("tlb_refill_check() succeed!\n");
}

void mips_init() {
	printk("init.c:\tmips_init() is called\n");

	mips_detect_memory();
	mips_vm_init();
	page_init();

	tlb_refill_check();
	halt();
}
