#include "types.h"
#include "x86.h"

#define SECTSIZE 512

#define DATA 0x1F0
#define FEATURES 0x1F1
#define COUNT 0x1F2
#define LOW 0x1F3
#define MID 0x1F4
#define HIGH 0x1F5
#define DRIVE 0x1F6
#define COMMAND 0x1F7
#define STATUS 0x1F7 /* Same as COMMAND */

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

    outb(0x1F6, 0xE0 | (lba >> 24));
    outb(0x1F2, count);
    outb(0x1F3, lba);
    outb(0x1F4, lba >> 8);
    outb(0x1F5, lba >> 16);
    outb(0x1F7, 0x20);  // READ SECTORS

    waitdisk();

    for (i = 0; i < count * SECTSIZE / 2; i++) {
        data = inw(DATA);
        *((ushort *)addr + i) = data;
    }
}
