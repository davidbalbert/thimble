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

int hello(int a, int b, int c, int d, int e, int f);

void
task1(void)
{
    for(;;) {
        hello(1, 2, 3, 4, 5, 6);
        wastetime();
    }
}

int goodbye(int a, int b, int c, int d, int e, int f);

void
task2(void)
{
    for(;;) {
        goodbye(6, 5, 4, 3, 2, 1);
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
