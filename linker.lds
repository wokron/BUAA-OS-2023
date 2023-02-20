/*
 * Set the architecture to mips.
 */
OUTPUT_ARCH(mips)

/*
 * Set the ENTRY point of the program to _start_mips.
 */
ENTRY(_start_mips)

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

	. = 0x80400000;
	end = . ;
}
