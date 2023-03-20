void range_1_check(void) {
	printk("%s%R%s", "This is a testcase: ", 2023, 2023, "\n");
	printk("the range is %R, size = %d\n", 1, 9, 9 - 1);
}

void mips_init() {
	range_1_check();
	halt();
}
