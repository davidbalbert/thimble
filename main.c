#include "types.h"

#include "common.h"
#include "irq.h"
#include "x86.h"

int
main(void)
{
    cclear();
    cprintf("Hello, Thimble!\n");

    trapinit();
    picinit();
    kbdinit();

    sti();

    for (;;)
        hlt();
}
