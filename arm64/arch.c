#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "uart.h"

#include "task1.h"

void
archinit_early(void)
{
    uart_init();
    cinit(uart_console);
}


void
archinit(void)
{
    cprintf("Exception Level: EL%d\n", el());

    mkproc(arm64_task1);
}
