OUTPUT_FORMAT(binary)

SECTIONS
{
	. = 0x7C00;

	.text :
	{
		*(.text);
		*(.ltext);
	}

	/DISCARD/ :
	{
		*(.*);
	}

	.bootsig 0x7DFE:
	{
		SHORT(0xAA55);
	}
}
