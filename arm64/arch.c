#include "u.h"

#include "archdefs.h"
#include "arm64.h"
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
