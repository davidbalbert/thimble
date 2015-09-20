#include "types.h"

#include "common.h"
#include "console.h"
#include "mem.h"
#include "x86.h"

int
main(void)
{
    cclear();
    cprintf("Hello, Thimble!");

    for (;;)
        hlt();
}
