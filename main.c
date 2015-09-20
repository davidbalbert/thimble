#include "types.h"

#include "mem.h"
#include "x86.h"

int
main(void)
{
    *((ushort *)0xB8000) = 0x5020;

    for (;;)
        hlt();
}
