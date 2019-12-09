#include "u.h"

// physical address -> bus address for SDRAM
u32
busaddr_mem(uintptr pa)
{
    return pa + 0xC0000000;
}

// physical address -> bus address for peripherals
u32
busaddr_p(uintptr pa)
{
    return 0x7E000000 | (0xFFFFFF & pa);
}
