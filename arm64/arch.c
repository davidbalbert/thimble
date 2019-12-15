#include "u.h"

#include "archdefs.h"
#include "lock.h"
#include "sleeplock.h"
#include "bio.h"
#include "defs.h"
#include "uart.h"

void
archinit_early(void)
{
    uart_init();
    cinit(uart_console);
}

void
archinit(void)
{
    sdinit();
}
