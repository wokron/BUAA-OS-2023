#define ENV_CREATE_USER(x, y, z)                                                                   \
	({                                                                                         \
		extern u_char binary_##x##_start[];                                                \
		extern u_int binary_##x##_size;                                                    \
		struct Env *tmp = env_create(binary_##x##_start, (u_int)binary_##x##_size, y);     \
		tmp->env_user = z;                                                                 \
	})

void mips_init() {
	printk("init.c:\tmips_init() is called\n");
	mips_detect_memory();
	mips_vm_init();
	page_init();

	env_init();

	// reverse insertion
	ENV_CREATE_USER(test_hash1, 1, 0);
	ENV_CREATE_USER(test_hash2, 2, 1);
	ENV_CREATE_USER(test_hash3, 3, 2);

	kclock_init();
	enable_irq();

	while (1) {
	}
}
