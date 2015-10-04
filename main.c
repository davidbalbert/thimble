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

    picenable(IRQ_KBD);
    sti();

    for (;;)
        hlt();
}
