#include "types.h"
#include "x86.h"

void
kmain(void)
{
    *((ushort *)0xB8000) = 0x1720;

    for (;;)
        hlt();
}
