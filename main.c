#include "types.h"

#include "common.h"
#include "irq.h"
#include "x86.h"

void
task1(void)
{
    int i = 0;

    for(;;) {
        cprintf("task1: %d\n", i++);
        yield();
    }
}

void
task2(void)
{
    int i = 0;

    for(;;) {
        cprintf("task2: %d\n", i++);
        yield();
    }
}


int
main(void)
{
    cclear();
    cprintf("Hello, Thimble!\n");

    trapinit();
    picinit();
    kbdinit();

    schedinit();
    start(task1);
    start(task2);

    sti();

    scheduler();
}
