void mips_init() {
	printk("init.c:\tmips_init() is called\n");

	mips_detect_memory();
	mips_vm_init();
	page_init();
	env_init();

	struct Env *ppa1 = ENV_CREATE_PRIORITY(test_ppa, 5);
	struct Env *ppa2 = ENV_CREATE_PRIORITY(test_ppa, 5);
	struct Env *ppa3 = ENV_CREATE_PRIORITY(test_ppa, 5);
	struct Env *ppa4 = ENV_CREATE_PRIORITY(test_ppa, 5);

	ppa2->env_parent_id = ppa1->env_id;
	ppa3->env_parent_id = ppa1->env_id;
	ppa4->env_parent_id = ppa3->env_id;
	kclock_init();
	enable_irq();
	while (1) {
	}
	panic("init.c:\tend of mips_init() reached!");
}
