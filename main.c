#include "u.h"

#include "defs.h"

int
main(void)
{
    archinit_console();

    cclear();
    cprintf("Hello, Thimble!\n");

    archmain();

    for (;;) {
        halt();
    }
}
