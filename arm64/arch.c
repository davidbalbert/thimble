#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "uart.h"

void
wastetime(void)
{
    ulong j;
    for (j = 0; j < 100000000; j++)
        ;
}

void
task1(void)
{
    int i = 0;

    for(;;) {
        cprintf("task1: %d\n", i++);
        wastetime();
    }
}

void
task2(void)
{
    int i = 0;

    for(;;) {
        cprintf("task2: %d\n", i++);
        wastetime();
    }
}

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

    start(task1);
    start(task2);
}
