void physical_memory_manage_strong_check(void) {
	struct Page *pp, *pp0, *pp1, *pp2, *pp3, *pp4;
	struct Page_list fl;
	int *temp1;

	// should be able to allocate three pages
	pp0 = pp1 = pp2 = pp3 = pp4 = 0;
	assert(page_alloc(&pp0) == 0);
	assert(page_alloc(&pp1) == 0);
	assert(page_alloc(&pp2) == 0);
	assert(page_alloc(&pp3) == 0);
	assert(page_alloc(&pp4) == 0);

	assert(pp0);
	assert(pp1 && pp1 != pp0);
	assert(pp2 && pp2 != pp1 && pp2 != pp0);
	assert(pp3 && pp3 != pp2 && pp3 != pp1 && pp3 != pp0);
	assert(pp4 && pp4 != pp3 && pp4 != pp2 && pp4 != pp1 && pp4 != pp0);

	// temporarily steal the rest of the free pages
	fl = page_free_list;
	// now this page_free list must be empty!!!!
	LIST_INIT(&page_free_list);
	// should be no free memory
	assert(page_alloc(&pp) == -E_NO_MEM);

	temp1 = (int *)page2kva(pp0);
	// write 1000 to pp0
	*temp1 = 1000;
	// free pp0
	page_free(pp0);
	printk("The number in address temp is %d\n", *temp1);

	// alloc again
	assert(page_alloc(&pp0) == 0);
	assert(pp0);

	// pp0 should not change
	assert(temp1 == (int *)page2kva(pp0));
	// pp0 should be zero
	assert(*temp1 == 0);

	page_free_list = fl;
	page_free(pp0);
	page_free(pp1);
	page_free(pp2);
	page_free(pp3);
	page_free(pp4);
	struct Page_list test_free;
	struct Page *test_pages;
	test_pages = (struct Page *)alloc(15 * sizeof(struct Page), BY2PG, 1);
	LIST_INIT(&test_free);
	int i, j = 0;
	struct Page *p, *q, *qq;
	for (i = 14; i >= 5; i--) {
		test_pages[i].pp_ref = i;
		LIST_INSERT_HEAD(&test_free, &test_pages[i], pp_link);
	}
	for (i = 0; i < 5; i++) {
		test_pages[i].pp_ref = i;
		LIST_INSERT_HEAD(&test_free, &test_pages[i], pp_link);
	}
	p = LIST_FIRST(&test_free);
	int answer1[] = {4, 3, 2, 1, 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
	assert(p != NULL);
	while (p != NULL) {
		assert(p->pp_ref == answer1[j++]);
		p = LIST_NEXT(p, pp_link);
	}
	// insert_after test
	int answer2[] = {4, 40, 20, 3, 2, 1, 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
	q = (struct Page *)alloc(sizeof(struct Page), BY2PG, 1);
	q->pp_ref = 20;
	qq = (struct Page *)alloc(sizeof(struct Page), BY2PG, 1);
	qq->pp_ref = 40;

	LIST_INSERT_AFTER(&test_pages[4], q, pp_link);
	LIST_INSERT_AFTER(&test_pages[4], qq, pp_link);
	p = LIST_FIRST(&test_free);
	j = 0;
	while (p != NULL) {
		assert(p->pp_ref == answer2[j++]);
		p = LIST_NEXT(p, pp_link);
	}
	printk("physical_memory_manage_check_strong() succeeded\n");
}

void mips_init() {
	printk("init.c:\tmips_init() is called\n");
	mips_detect_memory();
	mips_vm_init();
	page_init();

	physical_memory_manage_check();
	physical_memory_manage_strong_check();
	halt();
}
