OUTPUT_FORMAT(elf64-x86-64)
ENTRY(main)

SECTIONS
{
	. = 0xFFFF800000100000; /* KERNLINK */

	.text : AT(0x100000) {
		*(.text)
	}

	. = ALIGN(0x1000);

	.rodata : {
		*(.rodata)
	}

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}

	/DISCARD/ : {
		*(.*)
	}
}
