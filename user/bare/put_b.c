void _start() {
	for (unsigned i = 0;; ++i) {
		if ((i & ((1 << 16) - 1)) == 0) {
			// Requires `e->env_tf.cp0_status &= ~STATUS_KUp;` in kernel to work
			*(volatile char *)0xb0000000 = 'b';
			*(volatile char *)0xb0000000 = ' ';
		}
	}
}
