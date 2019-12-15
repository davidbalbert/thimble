#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "mem.h"

byte bootkstack[KSTACKSIZE] __attribute__ ((aligned (PGSIZE)));

void
halt(void)
{
    wfi();
}

void
intr_off(void)
{
    asm volatile("msr DAIFSet, #2");
}

void
intr_on(void)
{
    asm volatile("msr DAIFClr, #2");
}
