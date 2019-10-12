#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "uart.h"

void
archinit_console(void)
{
    uart_init();
    cinit(uart_console);
}


void
archmain(void)
{
    cprintf("Exception Level: EL%d\n", el());

    trapinit();

    for (;;) {
        cputc(uart_getc());
    }
}
