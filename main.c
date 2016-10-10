#include "u.h"

#include "defs.h"
#include "cpu.h"
#include "irq.h"
#include "mem.h"
#include "x86.h"

#include "task1.h"
#include "task2.h"

int
main(void)
{
    cclear();
    cprintf("Hello, Thimble!\n");

    schedinit();
    initmem1(end, p2v(4*MB));
    kvmalloc();
    seginit();
    trapinit();
    sysinit();
    picinit();
    timerinit();
    kbdinit();

    initmem2(p2v(4*MB), p2v(PHYSTOP));

    start(task1, task1_len);
    start(task2, task2_len);

    scheduler();
}
