#include "types.h"

#include "common.h"
#include "irq.h"
#include "x86.h"

#define PS2_DATA 0x60

void
kbdinit(void)
{
    picenable(IRQ_KBD);
}

void
handlekbd(void)
{
    uchar scan = inb(PS2_DATA);
    cprintf("%x ", scan);
}
