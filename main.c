#include "types.h"

#include "defs.h"
#include "cpu.h"
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

void hello(void);

void
task1(void)
{
    for(;;) {
        hello();
        wastetime();
    }
}

void goodbye(void);

void
task2(void)
{
    for(;;) {
        goodbye();
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
    seginit();
    trapinit();
    sysinit();
    picinit();
    timerinit();
    kbdinit();

    start(task1);
    start(task2);

    scheduler();
}
