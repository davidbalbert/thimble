#include "u.h"

#include "ata.h"
#include "bootdefs.h"
#include "x86.h"

static void
waitdisk(void)
{
    while((inb(0x1F7) & 0xC0) != 0x40)
        ;
}

void
ideread(byte *addr, u32 lba, byte sectcount)
{
    if (lba > ATA_MAXLBA28)
        panic("ideread - maxlba");

    waitdisk();

    outb(0x1F6, 0xE0 | (lba >> 24)); // primary drive + top 4 bytes of lba
    outb(0x1F2, sectcount);
    outb(0x1F3, lba);
    outb(0x1F4, lba >> 8);
    outb(0x1F5, lba >> 16);
    outb(0x1F7, ATA_CMD_READ_SECTORS);

    waitdisk();

    insw(0x1F0, addr, sectcount * ATA_SECTSIZE/2);
}
