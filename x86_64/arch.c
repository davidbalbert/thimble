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
    seginit();
    sysinit();
    picinit();
    timerinit();
    kbdinit();
    fileinit();

    initmem2(p2v(16*MB), p2v(PHYSTOP));

    mkproc(task1);

    scheduler();
}
