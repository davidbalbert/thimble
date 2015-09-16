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

void readlba(uchar *, ulong, ushort);

void
bootmain(void)
{
    readlba((uchar *)0x100000, 0, 1);

    for (;;)
        hlt();
}


void
waitdisk(void)
{
    while ((inb(STATUS) & 8) == 0)
        ;
}

void
readlba(uchar *addr, ulong start, ushort count)
{
    int i;
    ushort data;

    waitdisk();

    // Select LBA mode (are we also selecting the primary drive?)
    outb(DRIVE, 0x40);

    // For performance, don't send writes to the same IO port in a row.
    outb(COUNT, count >> 8);

    outb(LOW, start >> 24 & 0xFF); // Byte 4 (1 indexed)
    outb(MID, start >> 32 & 0xFF); // Byte 5
    outb(HIGH, start >> 40 & 0xFF); // Byte 6

    outb(COUNT, count & 0xFF);

    outb(LOW, start & 0xFF); // Byte 1
    outb(MID, start >> 8 & 0xFF); // Byte 2
    outb(HIGH, start >> 16 & 0xFF); // Byte 3

    outb(COMMAND, 0x24); // 0x24 == READ SECTORS EXT

    waitdisk();

    for (i = 0; i < count * SECTSIZE; i += 2) {
        data = inw(DATA);
        *(addr + i*2) = data;
    }
}
