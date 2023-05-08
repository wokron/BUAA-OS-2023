void mips_init() {
	printk("init.c:\tmips_init() is called\n");
	mips_detect_memory();
	mips_vm_init();
	page_init();
	env_init();
	ENV_CREATE_PRIORITY(test_test, 1);
	kclock_init();
	enable_irq();
	while (1) {
	}
	panic("init.c:\tend of mips_init() reached!");
}
