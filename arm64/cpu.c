#include "u.h"

#include "arm64.h"
#include "mem.h"

u8 bootkstack[KSTACKSIZE] __attribute__ ((aligned (PGSIZE)));

void
halt(void)
{
    wfi();
}
