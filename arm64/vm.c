#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "mem.h"
#include "proc.h"

static Pte *kpgmap;
static Pte *emptymap; // used to unmap all userspace addresses when we're in the scheduler.

// paging structure entry -> physical address
uintptr
pte_addr(Pte entry)
{
    return entry & 0x0000FFFFFFFFF000;
}

int
pte_flags(Pte entry)
{
    return entry & 0xFFFF000000000FFF;
}

int
uvmperm(void)
{
    return PTE_AF | PTE_ISH | PTE_CACHEABLE | PTE_W | PTE_U;
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

Pte *
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

int
mappages(Pte *pgmap, void *va, usize size, uintptr pa, int perm)
{
    char *a, *last;
    Pte *pte;

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

        if (a == last)
            break;
        a += PGSIZE;
        pa += PGSIZE;
    }

    return 0;
}

typedef struct Kmap Kmap;
static struct Kmap {
    void *addr;
    uintptr phys_start;
    uintptr phys_end;
    int perm;
} kmap[] = {
    {(void *)KERNBASE,            0,         V2P(data), PTE_AF | PTE_ISH | PTE_CACHEABLE | PTE_RO},     // kernel text and read only data
    {(void *)data,                V2P(data), PHYSTOP,   PTE_AF | PTE_ISH | PTE_CACHEABLE},              // kernel data + physical pages
    {(void *)(KERNBASE+DEVSPACE), DEVSPACE,  DEVTOP,    PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE},          // SoC peripherals
    {(void *)(KERNBASE+LSPACE),   LSPACE,    LTOP,      PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE}           // Local peripherals
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
    lttbr0(v2p(emptymap));

    dsb(); // Make sure page-table updates are done

    tlbi();

    dsb(); // make sure tlb invalidation is done
    isb();
}

void
switchuvm(Proc *p)
{
    pushcli();

    lttbr1(v2p(kpgmap));
    lttbr0(v2p(p->pgmap));

    dsb();
    tlbi();

    dsb();
    isb();

    popcli();
}

Pte *
allocpgmap(void)
{
    Pte *pgmap = kalloc();
    if (pgmap == nil)
        return nil;

    memzero(pgmap, PGSIZE);

    return pgmap;
}

void
kvmalloc(void)
{
    if ((kpgmap = setupkvm()) == nil) {
        panic("kvmalloc - kpgmap");
    }

    if ((emptymap = allocpgmap()) == nil) {
        panic("kvmalloc - emptymap");
    }

    switchkvm();
}

void
clearpteu(Pte *pgmap, void *addr)
{
    Pte *pte = walkpgmap(pgmap, addr, 0);
    if (pte == nil)
        panic("clearpteu");

    *pte &= ~PTE_U;
}
