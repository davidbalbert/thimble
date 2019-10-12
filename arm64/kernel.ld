OUTPUT_FORMAT(elf64-littleaarch64)
ENTRY(_start)

SECTIONS
{
    . = 0x80000; /* KERNPHYS - default address used by RPi boot loader */

    .text.boot : {
      *(.text.boot)
    }

    . += 0xFFFF000000000000; /* KERNBASE */

    .text : AT(text - 0xFFFF000000000000) {
      PROVIDE(text = .);
      *(.text)
      PROVIDE(etext = .);
    }

    .rodata : { *(.rodata) }
    .data   : { *(.data) }
    .bss    : { *(.bss COMMON) }

    PROVIDE(end = .);
}
