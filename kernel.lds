/*
 * Set the architecture to mips.
 */
OUTPUT_ARCH(mips)

/*
 * Set the ENTRY point of the program to _start.
 */
ENTRY(_start)

SECTIONS {
	/* Exercise 3.10: Your code here. */

	/* fill in the correct address of the key sections: text, data, bss. */
	/* Exercise 1.2: Your code here. */
	. = 0x80010000;
	.text : { *(.text) }
	.data : { *(.data) }
	.bss : { *(.bss) }

	bss_end = .;
	. = 0x80400000;
	end = . ;
}
