#include "u.h"

#include "defs.h"
#include "mem.h"

/*
static Pte *kpgmap;

typedef struct Kmap Kmap;
static struct Kmap {
    void *addr;
    uintptr phys_start;
    uintptr phys_end;
    int perm;
} kmap[] = {
    {(void *)KERNBASE,            0,         V2P(data), PTE_CACHEABLE | PTE_RO},    // kernel text and read only data
    {(void *)data,                V2P(data), PHYSTOP,   PTE_CACHEABLE | PTE_W},     // kernel data + physical pages
    {(void *)(KERNBASE+DEVSPACE), DEVSPACE,  DEVTOP,    PTE_DEVICE_nGnRnE | PTE_W } // MMIO peripherals
};

static Pte *
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
*/

void
kvmalloc(void)
{
    /*
    if ((kpgmap = setupkvm()) == nil)
        panic("kvmalloc");

    switchkvm();
    */
}
