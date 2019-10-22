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

static Pte *
coalesce(Pte *entry, Pte *end, int level)
{
    Pte *p;

    // never coalesce something with children
    if (level > 1 && *entry & PTE_TABLE) {
        return entry;
    }

    for (p = entry; (*p & 0xFFF) == (*entry & 0xFFF) && p < end; p++)
        ;

    return p-1;
}

static void
printmap0(Pte *pgdir, char *va, int level)
{
    Pte *entry = pgdir, *end = pgdir + 512, *innerdir, *lastinrange;
    usize mapsz = 4096l << (9 * (level-1));
    int i, j;

    while (entry < end) {
        if (!(*entry & PTE_P)) {
            va += mapsz;
            entry++;
            continue;
        }

        for (j = 4; j > level; j--) {
            cprintf("  ");
        }

        lastinrange = coalesce(entry, end, level);

        i = (uintptr)(entry-pgdir);
        j = (uintptr)(lastinrange-pgdir);

        cprintf("[0x%p-0x%p] ", va, va + (j-i+1) * mapsz - 1);

        if (i == j) {
            cprintf("PTL%d[%03d] ", level, i);
        } else {
            cprintf("PTL%d[%03d-%03d] ", level, i, j);
        }

        if (level == 1 && (*entry & PTE_PAGE)) {
            cprintf("PAGE ");

            printattrs(*entry);
            cprintf("0x%x\n", pte_addr(*entry));
        } else if (*entry & PTE_TABLE) {
            cprintf("TABLE\n");

            innerdir = p2v(pte_addr(*entry));
            printmap0(innerdir, va, level-1);
        } else {
            cprintf("BLOCK ");

            printattrs(*entry);
            cprintf("0x%x\n", pte_addr(*entry));
        }

        va += (j-i+1) * mapsz;
        entry = lastinrange + 1;
    }
}

void
printmap(Pte *pgdir)
{
    printmap0(pgdir, p2v(0), 4);
}
