#include "types.h"
#include "x86.h"
#include "mem.h"
#include "elf.h"

#define SECTSIZE 512

void readbytes(uchar *addr, ulong count, ulong offset);

void
stage2main(void)
{
    ElfHeader *elf;
    ElfProgHeader *ph, *eph;
    void (*entry)(void);
    uchar *pa;

    elf = (ElfHeader *)0x10000;
    readbytes((uchar *)elf, PGSIZE, 0);

    if (elf->magic != ELF_MAGIC) {
        for (;;)
            hlt();
    }

    ph = (ElfProgHeader *)((char *)elf + elf->phoff);
    eph = ph + elf->phnum;

    for (; ph < eph; ph++) {
        pa = (uchar *)ph->paddr;
        readbytes(pa, ph->filesz, ph->offset);
        if (ph->memsz > ph->filesz)
            stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
    }

    entry = (void(*)(void))elf->entry;
    entry();
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

void
readbytes(uchar *addr, ulong count, ulong offset)
{
    uchar *eaddr;
    uint lba;

    eaddr = addr + count;

    addr -= offset % SECTSIZE;
    lba = offset/SECTSIZE + 2; // kernel starts at sector 2. This assumes stage 2 is one sector wide

    for (; addr < eaddr; addr += SECTSIZE, lba++)
        readsects(addr, lba, 1);

}
