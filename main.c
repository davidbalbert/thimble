#include "types.h"

#include "common.h"
#include "irq.h"
#include "x86.h"

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
        yield();
    }
}

void
task2(void)
{
    int i = 0;

    for(;;) {
        cprintf("task2: %d\n", i++);
        wastetime();
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

    scheduler();
}
