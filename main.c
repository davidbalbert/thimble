#include "types.h"

#include "mem.h"
#include "x86.h"

int
main(void)
{
    *((ushort *)0xB8000) = 0x5020;

    for (;;)
        hlt();
}



// Paging structures used to map first 2MB at low and high
// addresses after we get control from the bootloader (see
// start.S)
__attribute__((__aligned__(PGSIZE)))
pml4e bootpml4[NPDENTRIES];


__attribute__((__aligned__(PGSIZE)))
pdpe bootpdphigh[NPDENTRIES];

__attribute__((__aligned__(PGSIZE)))
pde bootpgdirhigh[NPDENTRIES];


__attribute__((__aligned__(PGSIZE)))
pdpe bootpdplow[NPDENTRIES];

__attribute__((__aligned__(PGSIZE)))
pde bootpgdirlow[NPDENTRIES];

