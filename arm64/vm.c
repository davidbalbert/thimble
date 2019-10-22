#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "mem.h"

static Pte *kpgmap;
static Pte *emptymap; // used unmap all userspace addresses when we're in the scheduler.

static void
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

// paging structure entry -> physical address
uintptr
pte_addr(Pte entry)
{
    return entry & 0xFFFFFFFFF000;
}

static Pte *
pgmapget(Pte *table, int offset, int alloc)
{
    Pte *innertab;
    Pte *entry = &table[offset];

    if (*entry & PTE_P) {
        innertab = (ulong *)p2v(pte_addr(*entry));
    } else {
        if (!alloc || (innertab = kalloc()) == nil)
            return nil;

        memzero(innertab, PGSIZE);

        *entry = v2p(innertab) | PTE_TABLE | PTE_P;
    }

    return innertab;
}

// number of bits to shift in order to get the offset for the nth table entry.
// 0th entry isn't used.
static int shifts[] = { 0, 12, 21, 30, 39 };

static int
idx(void *va, int level)
{
    uintptr a = (uintptr)va;
    return (a >> shifts[level]) & 0x1FF;
}

static Pte *
walkpgmap(Pte *pgmap, void *va, int alloc)
{
    Pte *pgtab = pgmap;
    int i;

    for (i = 4; i > 1; i--) {
        pgtab = pgmapget(pgtab, idx(va, i), alloc);

        if (pgtab == nil)
            return nil;
    }

    return &pgtab[idx(va, 1)];
}

static int
mappages(Pte *pgmap, void *va, usize size, uintptr pa, int perm)
{
    char *a, *last;
    Pte *pte;

    cprintf("mappages(pgmap, va=0x%p, size=0x%x, pa=0x%x, perm)\n", va, size, pa);

    checkalign((void *)pa, PGSIZE, "mappages - physical addr not page aligned: pa = 0x%p", pa);

    a = pgfloor(va);
    last = pgfloor((void *)((uintptr)va + size - 1));

    for (;;) {
        pte = walkpgmap(pgmap, a, 1);

        if (pte == nil)
            return -1;
        if (*pte & PTE_P)
            panic("remap, va: 0x%x, pa: 0x%x", va, pa);

        *pte = pa | perm | PTE_PAGE | PTE_P;

        //cprintf("%l *walkpgmap(pgmap, 0x%p, 1) -> 0x%p\n", pgcount++, a, *pte);

        if (a == last)
            break;
        a += PGSIZE;
        pa += PGSIZE;
    }

    return 0;
}



/*
typedef struct Kmap Kmap;
static struct Kmap {
    void *addr;
    uintptr phys_start;
    uintptr phys_end;
    int perm;
} kmap[] = {
    {(void *)KERNBASE,            0,         V2P(data), PTE_AF | PTE_ISH | PTE_CACHEABLE},    // kernel text and read only data
    {(void *)data,                V2P(data), PHYSTOP,   PTE_AF | PTE_ISH | PTE_CACHEABLE},     // kernel data + physical pages
    {(void *)(KERNBASE+DEVSPACE), DEVSPACE,  DEVTOP,    PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE} // MMIO peripherals
};
*/

typedef struct Kmap Kmap;
static struct Kmap {
    void *addr;
    uintptr phys_start;
    uintptr phys_end;
    int perm;
} kmap[] = {
    {(void *)KERNBASE,            0,         18*MB,     PTE_AF | PTE_ISH | PTE_CACHEABLE},
    {(void *)(KERNBASE+DEVSPACE), DEVSPACE,  DEVTOP,    PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE}
};

Pte *
setupkvm()
{
    Pte *pgmap = kalloc();
    Kmap *k;

    if (pgmap == nil)
        return nil;

    memzero(pgmap, PGSIZE);

    for (k = kmap; k < &kmap[nelem(kmap)]; k++) {
        if (mappages(pgmap, k->addr, k->phys_end-k->phys_start, k->phys_start, k->perm) < 0)
            return nil;
    }

    return pgmap;
}

void
switchkvm(void)
{
    lttbr1(v2p(kpgmap));
    //lttbr0(v2p(emptymap));

    dsbsy(); // probably could be dsb ish, but I need to learn more about how inner/outer sharable work first.

    tlbi();

    dsbsy(); // ditto
    isb();
}

void
printmaps(void)
{
    uintptr pcurrent;
    Pte *current;//, *entry;
    asm volatile("mrs %[pcurrent], ttbr1_el1" : [pcurrent] "=r" (pcurrent));

    current = p2v(pcurrent);

    cprintf("ttbr1_el1=0x%p\n", current);
    cprintf("kpgmap=   0x%p\n", kpgmap);
    cprintf("emptymap= 0x%p\n", emptymap);
    cprintf("\n\n");

    cprintf("current:\n");
    printmap(current);

    cprintf("\n\nkpgmap:\n");
    printmap(kpgmap);
}

void
kvmalloc(void)
{
    if ((kpgmap = setupkvm()) == nil) {
        panic("kvmalloc - kpgmap");
    }

    if ((emptymap = kalloc()) == nil) {
        panic("kvmalloc - emptymap");
    }

    memzero(emptymap, PGSIZE);

    printmaps();
    //switchkvm();
}
