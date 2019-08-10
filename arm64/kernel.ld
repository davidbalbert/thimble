OUTPUT_FORMAT(elf64-littleaarch64)
ENTRY(_start)

SECTIONS
{
    . = 0x80000; /* default address used by RPi boot loader */

    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data   : { *(.data) }
    .bss    : { *(.bss COMMON) }

    PROVIDE(STACKBASE = .);
    . += 0x10000;
    PROVIDE(STACKTOP = .);
}
