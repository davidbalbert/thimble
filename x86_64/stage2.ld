OUTPUT_FORMAT(binary)

SECTIONS
{
	. = 0x7E00;

	.text :
	{
		*(.text);
		*(.ltext);
	}

	.rodata :
	{
		*(.rodata*);
	}

	.bss :
	{
		*(.bss);
		*(.lbss);
	}

	.data :
	{
		*(.data);
		*(.ldata);
	}

	/DISCARD/ :
	{
		*(.*);
	}
}
