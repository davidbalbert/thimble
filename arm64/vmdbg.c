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

        next = coalesce(entry, end, level);

        i = (uintptr)(entry-pgdir);
        j = (uintptr)(next-pgdir);

        cprintf("[0x%p-0x%p] ", va, va + (j-i) * mapsz - 1);

        if (i == j - 1) {
            cprintf("PTL%d[%03d] ", level, i);
        } else {
            cprintf("PTL%d[%03d-%03d] ", level, i, j - 1);
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

        va += (j-i) * mapsz;
        entry = next;
    }
}

void
printmap(Pte *pgdir)
{
    printmap0(pgdir, p2v(0), 4);
}
