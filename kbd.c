#include "types.h"

#include "common.h"
#include "irq.h"

void
kbdinit(void)
{
    picenable(IRQ_KBD);
}

void
handlekbd(void)
{
}
