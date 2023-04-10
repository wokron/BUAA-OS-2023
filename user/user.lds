/*
 * Set the architecture to mips.
 */
OUTPUT_ARCH(mips)

/*
 * Set the ENTRY point of the program to _start.
 */
ENTRY(_start)

PHDRS {
	code PT_LOAD FLAGS (5);
	data PT_LOAD FLAGS (6);
}

SECTIONS {
	. = 0x00400000;

	.text : {
		*(.text)
		*(.rodata)
		*(.rodata.*)
	} : code

	.data ALIGN(4096) : {
		*(.data)
	} : data

	.bss ALIGN(4096) : {
		*(.bss)
	} : data

	end = . ;
}
