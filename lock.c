#include "types.h"

#include "cpu.h"
#include "common.h"
#include "lock.h"
#include "proc.h"
#include "x86.h"

void
initlock(SpinLock *l)
{
    l->locked = 0;
}

void
lock(SpinLock *l)
{
    pushcli();

    while(xchg(&l->locked, 1) == 1)
        ;
}

void
unlock(SpinLock *l)
{
    xchg(&l->locked, 0);
    popcli();
}

void
pushcli(void)
{
    // TODO: save interrupt enabled on first push
    ulong rflags = readrflags();

    cli();
    if (cpu->ncli++ == 0)
        cpu->intena = rflags & FL_IF;
}

void
popcli(void)
{
    if (--cpu->ncli < 0)
        panic("popcli");

    if (cpu->ncli == 0 && cpu->intena)
        sti();
}
