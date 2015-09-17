#include "types.h"
#include "x86.h"

#define SECTSIZE 512

void readlba(void *, uint, uchar);

void
bootmain(void)
{
    readlba((void *)0x100000, 1, 1);

    for (;;)
        hlt();
}


void
waitdisk(void)
{
    while((inb(0x1F7) & 0xC0) != 0x40)
        ;
}

void
readlba(void *addr, uint lba, uchar count)
{
    int i;
    ushort data;

    waitdisk();

    outb(0x1F6, 0xE0 | (lba >> 24)); // primary drive + top 4 bytes of lba
    outb(0x1F2, count);
    outb(0x1F3, lba);
    outb(0x1F4, lba >> 8);
    outb(0x1F5, lba >> 16);
    outb(0x1F7, 0x20);  // read sectors

    waitdisk();

    insw(0x1F0, addr, count * SECTSIZE/2);
}
