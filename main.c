#include "u.h"

#include "defs.h"
#include "mem.h"

#include "task1.h"

int
main(void)
{
    archinit_early();

    cclear();
    cprintf("Hello, Thimble!\n");

    schedinit();
    // TODO: allocate fewer things (files, etc.) statically in the
    // kernel so we need to map less memory initially
    initmem1(end, p2v(16*MB));
    kvmalloc();
    trapinit();
    intinit();
    timerinit();
    fileinit();

    initmem2(p2v(16*MB), p2v(PHYSTOP));

    archinit();

    mkproc(task1);

    scheduler();
}
