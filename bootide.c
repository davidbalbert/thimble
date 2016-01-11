#include "types.h"

#include "x86.h"

#define SECTSIZE 512

static void
waitdisk(void)
{
    while((inb(0x1F7) & 0xC0) != 0x40)
        ;
}

void
ideread(uchar *addr, uint lba, uchar sectcount)
{
    waitdisk();

    outb(0x1F6, 0xE0 | (lba >> 24)); // primary drive + top 4 bytes of lba
    outb(0x1F2, sectcount);
    outb(0x1F3, lba);
    outb(0x1F4, lba >> 8);
    outb(0x1F5, lba >> 16);
    outb(0x1F7, 0x20);  // read sectors

    waitdisk();

    insw(0x1F0, addr, sectcount * SECTSIZE/2);
}
