#include "u.h"

#include "defs.h"
#include "mem.h"
#include "proc.h"

void
checkalign(void *a, int alignment, char *msg, ...)
{
    uintptr aa = (uintptr)a;
    va_list ap;

    if (aa & (alignment-1)) {
        va_start(ap, msg);
        vpanic(msg, ap);
        va_end(ap);
    }
}

Pte *
copyuvm(Pte *oldmap, usize sz)
{
    uintptr a;
    Pte *newmap;
    Pte *pte;
    byte *oldmem, *newmem;
    u32 flags;

    newmap = allocpgmap();
    if (newmap == nil)
        return nil;

    for (a = 0; a < sz; a += PGSIZE) {
        pte = walkpgmap(oldmap, (void *)a, 0);
        if (pte == nil)
            panic("copyuvm - nil pte");
        if (!*pte & PTE_P)
            panic("copyuvm - page not present");

        oldmem = p2v(pte_addr(*pte));
        flags = pte_flags(*pte);

        newmem = kalloc();
        if (newmem == nil)
            goto bad;

        memmove(newmem, oldmem, PGSIZE);

        if (mappages(newmap, (void *)a, PGSIZE, v2p(newmem), flags) < 0)
            goto bad;
    }

    return newmap;

bad:
    freeuvm(newmap);
    return nil;
}

void
freeuvm(Pte *pgmap)
{
    cprintf("freeuvm not implemented yet!\n");
    // todo
}

usize
allocuvm(Pte *pgmap, usize oldsz, usize newsz)
{
    void *mem;
    usize a;

    if (newsz >= USERTOP)
        return 0;
    if (newsz < oldsz)
        return oldsz;

    a = (usize)pgceil((void *)oldsz);

    for (; a < newsz; a += PGSIZE) {
        mem = kalloc();
        if (mem == nil) {
            cprintf("allocuvm -- oom (should dealloc here)\n");
            return 0;
        }

        memzero(mem, PGSIZE);

        if (mappages(pgmap, (char *)a, PGSIZE, v2p(mem), uvmperm()) < 0) {
            cprintf("allocuvm -- oom 2 (should dealloc here)\n");
            kfree(mem);
            return 0;
        }
    }

    return newsz;
}

void
loaduvm(Pte *pgmap, char *addr, byte *data, u64 sz)
{
    Pte *pte;
    u64 i;
    usize n;
    char *pg;

    checkalign(addr, PGSIZE, "loaduvm - addr not page aligned");

    for (i = 0; i < sz; i+= PGSIZE) {
        pte = walkpgmap(pgmap, addr+i, 0);
        if (pte == nil)
            panic("loaduvm - addr not mapped");

        pg = p2v(pte_addr(*pte));

        if (sz - i < PGSIZE)
            n = sz - i;
        else
            n = PGSIZE;

        memmove(pg, data + i, n);
    }
}
