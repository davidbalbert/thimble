#include "u.h"

#include "defs.h"
#include "mem.h"

int
main(void)
{
    archinit_console();

    cclear();
    cprintf("Hello, Thimble!\n");

    schedinit();
    // TODO: allocate fewer things (files, etc.) statically in the
    // kernel so we need to map less memory initially
    initmem1(end, p2v(16*MB));
    kvmalloc();

    archmain();

    for (;;) {
        halt();
    }
}
