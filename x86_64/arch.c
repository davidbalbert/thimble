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
    kbdinit();
    fileinit();

    mkproc(task1);

    scheduler();
}
