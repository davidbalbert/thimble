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
    genint(32);

    for (;;)
        hlt();
}
