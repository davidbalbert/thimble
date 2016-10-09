#include "u.h"

#include "bootdefs.h"
#include "elf.h"
#include "mem.h"
#include "pci.h"
#include "x86.h"

#define SECTSIZE 512

static int has_ahci = 0; // Set if we find an ahci controller

// required by the console driver
void
panic(char *fmt, ...)
{
    va_list ap;
    cprintf("bootpanic: ");

    va_start(ap, fmt);
    cvprintf(fmt, ap);
    va_end(ap);

    cputc('\n');

    for (;;)
        hlt();
}

void readbytes(uchar *addr, ulong count, ulong offset);

// koffset is the first byte of the kernel on disk
void
stage2main(ulong koffset)
{
    ElfHeader *elf;
    ElfProgHeader *ph, *eph;
    void (*entry)(void);
    uchar *pa;

    cclear();

    has_ahci = ahcidetect();

    elf = (ElfHeader *)0x10000;
    readbytes((uchar *)elf, PGSIZE, koffset);

    if (elf->magic != ELF_MAGIC)
        panic("stage2main - elf magic");

    ph = (ElfProgHeader *)((char *)elf + elf->phoff);
    eph = ph + elf->phnum;

    for (; ph < eph; ph++) {
        pa = (uchar *)ph->paddr;
        readbytes(pa, ph->filesz, koffset + ph->offset);
        if (ph->memsz > ph->filesz)
            stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
    }

    entry = (void(*)(void))elf->entry;
    entry();
}

// addr - destination
// count - bytes to read
// offset - start byte (offset from start of disk)
void
readbytes(uchar *addr, ulong count, ulong offset)
{
    uchar *eaddr;
    uint lba;

    eaddr = addr + count;

    addr -= offset % SECTSIZE;
    lba = offset/SECTSIZE;

    for (; addr < eaddr; addr += SECTSIZE, lba++) {
        if (has_ahci) {
            ahciread(addr, lba, 1);
        } else {
            ideread(addr, lba, 1);
        }
    }
}
