#include "u.h"

#include "archdefs.h"
#include "cpu.h"
#include "defs.h"
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
    push_off();

    while(xchg(&l->locked, 1) == 1)
        ;
}

void
unlock(SpinLock *l)
{
    xchg(&l->locked, 0);
    pop_off();
}

void
push_off(void)
{
    ulong rflags = readrflags();

    intr_off();
    if (cpu->noff++ == 0)
        cpu->intena = rflags & FL_IF;
}

void
pop_off(void)
{
    if (--cpu->noff < 0)
        panic("pop_off");

    if (cpu->noff == 0 && cpu->intena)
        intr_on();
}
