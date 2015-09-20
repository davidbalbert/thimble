OUTPUT_FORMAT(elf64-x86-64)
ENTRY(main)

SECTIONS
{
	. = 0xFFFF800000100000; /* KERNLINK */

	.text : AT(0x100000) {
		*(.text)
	}

	.rodata : {
		*(.rodata)
	}

	. = ALIGN(0x1000);

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
