#include "u.h"

#include "defs.h"
#include "cpu.h"
#include "irq.h"
#include "mem.h"

#include "arm64/uart.h"
#include "arm64/arm64.h"

//#include "task1.h"


void
early_uart_console(void)
{
    uart_init();
    cinit(uart_console);
}

int
main(void)
{
    while (1);
    //uart_init();
    //cinit(uart_console);
    cclear();
    cprintf("Hello, Thimble!\n");
    cprintf("Exception Level: EL%d\n", el());

    asm volatile("mrs x0, elr_el3");

    for (;;) {
        cputc(uart_getc());
    }

    /*
    schedinit();
    // TODO: allocate fewer things (files, etc.) statically in the
    // kernel so we need to map less memory initially
    initmem1(end, p2v(16*MB));
    kvmalloc();
    seginit();
    trapinit();
    sysinit();
    picinit();
    timerinit();
    kbdinit();
    fileinit();

    initmem2(p2v(16*MB), p2v(PHYSTOP));

    scheduler();
    */

    for (;;) {
        halt();
    }
}
