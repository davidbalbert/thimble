OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
	. = 0xFFFF800000100000; /* KERNLINK */

	.text : AT(0x100000) {
		*(.text)
	}

	. = ALIGN(0x1000);

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}

	/DISCARD/ : {
		*(.eh_frame)
	}
}
