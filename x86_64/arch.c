#include "u.h"

#include "defs.h"
#include "mem.h"

#include "task1.h"

void
archinit_console(void)
{
    cinit(vga_console);
}

void
archmain(void)
{
    schedinit();
    // TODO: allocate fewer things (files, etc.) statically in the
    // kernel so we need to map less memory initially
    initmem1(end, p2v(16*MB));
    kvmalloc();
    seginit();
    trapinit();
    sysinit();
    picinit();
    timerinit();
    kbdinit();
    fileinit();

    initmem2(p2v(16*MB), p2v(PHYSTOP));

    mkproc(task1);

    scheduler();
}
