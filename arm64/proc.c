#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "lock.h"
#include "proc.h"

#define NPROCS 1024

static struct {
    SpinLock lock;
    Proc procs[NPROCS];
} ptable;

int nextpid = 1;

#define MAXCPU 8

static Cpu cpus[MAXCPU];

Cpu  *cpu;  // current cpu

void
mkproc(uchar *data)
{
    Proc *p;
    uchar *usp;     // user virtual address of user stack pointer
    uchar *ustack;  // kernel virtual address of user stack pointer
    ElfHeader *elf;
    ElfProgHeader *ph, *eph;
}

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
    initlock(&ptable.lock);

    cpu = &cpus[0];
    cpu->id = 0;
    cpu->ncli = 0;
}
