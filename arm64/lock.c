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
    pushcli();

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
    popcli();
}

void
pushcli(void)
{
    u64 daif = readdaif();

    cli();

    if (cpu->ncli++ == 0) {
        cpu->intena = (daif & DAIF_I) == 0;
    }
}

void
popcli(void)
{
    if (--cpu->ncli < 0) {
        panic("popcli");
    }

    if (cpu->ncli == 0 && cpu->intena) {
        sti();
    }
}
