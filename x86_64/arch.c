#include "u.h"

#include "archdefs.h"
#include "defs.h"
#include "mem.h"

#include "task1.h"

void
archinit_early(void)
{
    cinit(vga_console);
}

void
archinit(void)
{
    seginit();
    sysinit();
    kbdinit();
    fileinit();

    mkproc(task1);
}
