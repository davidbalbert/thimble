#include "u.h"

#include "arm64.h"
#include "asm.h"
#include "defs.h"
#include "lock.h"
#include "proc.h"

void
initlock(SpinLock *l)
{
    l->locked = 0;
}

void
lock(SpinLock *l)
{
    push_off();

    while (1) {
        while (ldaxr(&l->locked)) {
            wfe();
        }

        if (stxr(&l->locked, 1) == 0) {
            break;
        }

        wfe();
    }
}

void
unlock(SpinLock *l)
{
    stlr(&l->locked, 0);
    pop_off();
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
