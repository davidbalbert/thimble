#include "types.h"

#include "elf.h"
#include "mem.h"
#include "x86.h"

#include "bootide.h"

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
