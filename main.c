#include "types.h"

#include "console.h"
#include "mem.h"
#include "x86.h"

void cscroll(void);

int
main(void)
{
    cclear();
    cputs("Hello, Thimble!");

    for (;;)
        hlt();
}
