void mips_init() {
	printk("init.c:\tmips_init() is called\n");
	mips_detect_memory();
	mips_vm_init();
	page_init();
	env_init();

	envid2env_check();
	halt();
}
