#include "types.h"

#include "common.h"
#include "irq.h"
#include "mem.h"
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

int
main(void)
{
    cclear();
    cprintf("Hello, Thimble!\n");

    initmem1(end, p2v(2 * 1024 * 1024));
    schedinit();
    trapinit();
    picinit();
    timerinit();
    kbdinit();

    //start(task1);
    //start(task2);

    scheduler();
}
