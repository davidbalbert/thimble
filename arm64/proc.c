#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "proc.h"

#define MAXCPU 8

static Cpu cpus[MAXCPU];

Cpu  *cpu;  // current cpu

void
panic(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vpanic(fmt, ap);
    va_end(ap);
}

void
vpanic(char *fmt, va_list ap)
{
    cli();
    cprintf("cpu%d: panic: ", cpu->id);

    cvprintf(fmt, ap);

    cputc('\n');

    for (;;)
        halt();
}

void
schedinit(void)
{
    //initlock(&ptable.lock);

    cpu = &cpus[0];
    cpu->id = 0;
    cpu->ncli = 0;
}
