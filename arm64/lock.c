#include "u.h"

#include "arm64.h"
#include "lock.h"

void
initlock(SpinLock *l)
{
    l->locked = 0;
}

void
lock(SpinLock *l)
{
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
}

void
pushcli(void)
{

}

void
popcli(void)
{

}
