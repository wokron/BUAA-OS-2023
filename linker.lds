/*
 * Set the architecture to mips.
 */
OUTPUT_ARCH(mips)

/*
 * Set the ENTRY point of the program to _start.
 */
ENTRY(_start)

SECTIONS {
	. = 0x80010000;

	.text : {
		*(.text)
	}

	.data : {
		*(.data)
	}

	.bss  : {
		*(.bss)
	}
}
