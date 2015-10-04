#include "types.h"

#include "common.h"
#include "mem.h"
#include "x86.h"

int
main(void)
{

    cclear();
    cprintf("Hello, Thimble!\n");

    initidt();

    cprintf("before\n");
    INT(50);
    cprintf("middle\n");
    INT(32);
    cprintf("after\n");

    for (;;)
        hlt();
}
