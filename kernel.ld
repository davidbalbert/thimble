OUTPUT_FORMAT(elf64-x86-64)
ENTRY(main)

SECTIONS
{
	. = 0xFFFF800000100000; /* KERNLINK */

	.text : {
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
		*(.eh_frame)
	}
}
