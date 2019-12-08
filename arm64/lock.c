#include "u.h"

#include "arm64.h"
#include "asm.h"
#include "defs.h"
#include "lock.h"
#include "proc.h"

void
initlock(SpinLock *l, char *name)
{
    l->locked = 0;
    l->name = name;
    l->cpu = nil;
}

void
lock(SpinLock *l)
{
    push_off();

    if (holding(l)) {
        panic("lock");
    }

    while (1) {
        while (ldaxr(&l->locked)) {
            wfe();
        }

        if (stxr(&l->locked, 1) == 0) {
            break;
        }

        wfe();
    }

    l->cpu = mycpu();
}

void
unlock(SpinLock *l)
{
    if (!holding(l)) {
        panic("unlock");
    }

    l->cpu = nil;

    stlr(&l->locked, 0);
    pop_off();
}

int
holding(SpinLock *l)
{
    int r;

    push_off();
    r = l->locked && l->cpu == mycpu();
    pop_off();

    return r;
}

void
push_off(void)
{
    u64 daif = readdaif();

    intr_off();

    if (cpu->noff++ == 0) {
        cpu->intena = (daif & DAIF_I) == 0;
    }
}

void
pop_off(void)
{
    if (--cpu->noff < 0) {
        panic("pop_off");
    }

    if (cpu->noff == 0 && cpu->intena) {
        intr_on();
    }
}
