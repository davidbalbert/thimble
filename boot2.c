#include "x86.h"

void
bootmain(void)
{
    int i;
    short *vmem = (short *)0xB8000;

    for (i = 0; i < 80 * 25; i++)
        vmem[i] = 0x1720;

    for (;;)
        hlt();
}
