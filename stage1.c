#include "types.h"
#include "mem.h"
#include "x86.h"
#include "elf.h"

#define SECTSIZE 512

void readsects(uchar *addr, uint lba, uchar count);

void
bootmain(void)
{
    void *addr;
    void (*stage2start)(void);

    addr = (void *)0x7E00;
    readsects(addr, 1, 1);
    
    stage2start = (void(*)(void))addr;
    stage2start();
}

void
waitdisk(void)
{
    while((inb(0x1F7) & 0xC0) != 0x40)
        ;
}

void
readsects(uchar *addr, uint lba, uchar count)
{
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
