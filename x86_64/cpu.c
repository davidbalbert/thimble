#include "u.h"

#include "x86.h"

void
halt(void)
{
    hlt();
}

void
intr_off(void)
{
    asm volatile("cli");
}

void
intr_on(void)
{
    asm volatile("sti");
}
