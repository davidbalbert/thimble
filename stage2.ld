OUTPUT_FORMAT(binary)

SECTIONS
{
	. = 0x7E00;

	.text :
	{
		*(.text);
	}

	.rodata :
	{
		*(.rodata);
	}

	.bss :
	{
		*(.bss);
	}

	.data :
	{
		*(.data);
	}

	/DISCARD/ :
	{
		*(.*);
	}
}
