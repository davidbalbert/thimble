#include "u.h"

#include "defs.h"
#include "mem.h"

uintptr pte_addr(Pte entry);

static void
printattrs(Pte entry)
{
    if (entry & PTE_CACHEABLE) {
        cprintf("C  ");
    } else if (entry & PTE_NON_CACHEABLE) {
        cprintf("NC ");
    } else if (entry & PTE_DEVICE_GRE) {
        cprintf("D  ");
    } else {
        cprintf("Dn ");
    }

    if (entry & PTE_AF) {
        cprintf("A");
    } else {
        cprintf("-");
    }

    if (entry & PTE_ISH) {
        cprintf("I");
    } else {
        cprintf("-");
    }

    if (entry & PTE_RO) {
        cprintf("R");
    } else {
        cprintf("W");
    }

    if (entry & PTE_U) {
        cprintf("U ");
    } else {
        cprintf("- ");
    }
}

// page table level -> # of bytes mapped by an entry at that level
static usize
mapsz(int level)
{
    return 4096l << (9 * (level-1));
}

// returns the first PTE that doesn't map a physical address contiguous to the
// physical address mapped by entry
static Pte *
coalesceaddr(Pte *entry, Pte *end, int level)
{
    Pte *p;
    usize sz = mapsz(level);

    for (p = entry+1; pte_addr(*(p-1)) + sz == pte_addr(*p) && p < end; p++)
        ;

    return p;
}


static void
printaddrs(Pte *entry, Pte *end, int level)
{
    Pte *next, *last;
    usize sz = mapsz(level);

    while (entry < end) {
        next = coalesceaddr(entry, end, level);

        last = next-1;
        cprintf("0x%x-0x%x ", pte_addr(*entry), pte_addr(*last) + sz - 1);

        entry = next;
    }
}

// skips all PTEs with the same attributes as entry. Returns the first PTE with
// different attributes.
static Pte *
coalesce(Pte *entry, Pte *end, int level)
{
    Pte *p;

    // Never coalesce something with children. PTE_TABLE and PTE_PAGE are the
    // same value, so we have to check level as well.
    if (level > 1 && *entry & PTE_TABLE) {
        return entry+1;
    }

    for (p = entry; (*p & 0xFFF) == (*entry & 0xFFF) && p < end; p++)
        ;

    return p;
}

static void
printmap0(Pte *pgdir, char *va, int level)
{
    Pte *entry = pgdir, *end = pgdir + 512, *innerdir, *next;
    usize sz = mapsz(level);
    int i, j;

    while (entry < end) {
        if (!(*entry & PTE_P)) {
            va += sz;
            entry++;
            continue;
        }

        for (j = 4; j > level; j--) {
            cprintf("  ");
        }

        next = coalesce(entry, end, level);

        i = (uintptr)(entry-pgdir);
        j = (uintptr)(next-pgdir);

        cprintf("[0x%016p-0x%016p] ", va, va + (j-i) * sz - 1);

        if (i == j - 1) {
            cprintf("PTL%d[%03d] ", level, i);
        } else {
            cprintf("PTL%d[%03d-%03d] ", level, i, j - 1);
        }

        if (level == 1 && (*entry & PTE_PAGE)) {
            cprintf("PAGE ");

            printattrs(*entry);
            printaddrs(entry, next, level);
            cprintf("\n");
        } else if (*entry & PTE_TABLE) {
            cprintf("TABLE\n");

            innerdir = p2v(pte_addr(*entry));
            printmap0(innerdir, va, level-1);
        } else {
            cprintf("BLOCK ");

            printattrs(*entry);
            printaddrs(entry, next, level);
            cprintf("\n");
        }

        va += (j-i) * sz;
        entry = next;
    }
}

void
kprintmap(Pte *pgdir)
{
    printmap0(pgdir, p2v(0), 4);
}

void
uprintmap(Pte *pgdir)
{
    printmap0(pgdir, 0, 4);
}
