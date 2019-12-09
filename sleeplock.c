#include "u.h"

#include "defs.h"
#include "lock.h"
#include "sleeplock.h"

void
initsleeplock(SleepLock *l, char *name)
{
    initlock(&l->lock, "sleep lock");
    l->locked = 0;
    l->name = name;
    l->pid = 0;
}

void
locksleep(SleepLock *l)
{
    lock(&l->lock);
    while (l->locked) {
        sleep(&l, &l->lock);
    }

    l->locked = 1;
    // l->pid = myproc()->pid;
    unlock(&l->lock);
}

void
unlocksleep(SleepLock *l)
{
    if (!holdingsleep(l)) {
        panic("unlocksleep");
    }

    lock(&l->lock);
    l->locked = 0;
    l->pid = 0;
    wakeup(l);
    unlock(&l->lock);
}

int
holdingsleep(SleepLock *l)
{
    int r;
    lock(&l->lock);
    r = l->locked == 1 /* && l->pid == myproc()->pid */;
    unlock(&l->lock);
    return r;
}
