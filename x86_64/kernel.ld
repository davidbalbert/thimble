OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

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

	PROVIDE(data = .);

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}

	PROVIDE(end = .);

	/DISCARD/ : {
		*(.eh_frame)
	}
}
