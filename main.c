#include "u.h"

#include "defs.h"
#include "cpu.h"
#include "irq.h"
#include "mem.h"

#include "arm64/uart.h"

//#include "task1.h"


/*
char uart_getc(void);
*/

void vcputc(uchar c);

int
main(void)
{
    cinit(uart_console);
    cclear();
    cprintf("Hello, Thimble!\n");

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
