#include "u.h"

#include "defs.h"
#include "cpu.h"
#include "irq.h"
#include "mem.h"
#include "x86.h"

#include "task1.h"
#include "task2.h"

int
main(void)
{
    cclear();
    cprintf("Hello, Thimble!\n");

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

    initmem2(p2v(16*MB), p2v(PHYSTOP));

    mkproc(task1);
    mkproc(task2);

    scheduler();
}
