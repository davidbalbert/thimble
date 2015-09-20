#include "types.h"

#include "mem.h"
#include "x86.h"

void
kmain(void)
{
    *((ushort *)0xB8000) = 0x4020;

    for (;;)
        hlt();
}


__attribute__((__aligned__(PGSIZE)))
pde bootpgdirhigh[NPDENTRIES];

__attribute__((__aligned__(PGSIZE)))
pdpe bootpdphigh[NPDENTRIES];


__attribute__((__aligned__(PGSIZE)))
pde bootpgdirlow[NPDENTRIES];

__attribute__((__aligned__(PGSIZE)))
pdpe bootpdplow[NPDENTRIES];

__attribute__((__aligned__(PGSIZE)))
pml4e bootpml4[NPDENTRIES];

