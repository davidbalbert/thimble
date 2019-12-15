#include "u.h"

#include "archdefs.h"
#include "defs.h"
#include "lock.h"
#include "sleeplock.h"

#include "bio.h"

#define NBUF 30

struct {
    SpinLock lock;
    Buf bufs[NBUF];
} bcache;

void
binit(void)
{
    Buf *b;

    initlock(&bcache.lock, "bcache");

    for (b = bcache.bufs; b < bcache.bufs+NBUF; b++) {
        initsleeplock(&b->lock, "buf");
    }
}

Buf *
bget(uint dev, u64 blockno)
{
    Buf *b;

    lock(&bcache.lock);

    // first, see if the buffer is already cached
    for (b = bcache.bufs; b < bcache.bufs+NBUF; b++) {
        if (b->dev == dev && b->blockno == blockno) {
            b->refcnt++;
            unlock(&bcache.lock);
            locksleep(&b->lock);
            return b;
        }
    }

    // if we don't find it, allocate a new buffer
    for (b = bcache.bufs; b < bcache.bufs+NBUF; b++) {
        if (b->refcnt == 0) {
            b->dev = dev;
            b->blockno = blockno;
            b->refcnt = 1;
            b->valid = 0;
            unlock(&bcache.lock);
            locksleep(&b->lock);
            return b;
        }
    }

    panic("bget - out of buffers");
}

// returns a locked buf
Buf *
bread(uint dev, u64 blockno)
{
    Buf *b;

    b = bget(dev, blockno);

    if (!b->valid) {
        sdrw(b, 0);
        b->valid = 1;
    }

    return b;
}

void
brelse(Buf *b)
{
    unlocksleep(&b->lock);

    lock(&bcache.lock);
    b->refcnt--;
    unlock(&bcache.lock);
}

void bwrite(Buf *b)
{
    panic("not implemented");
}

// long bread(uint dev, u64 blockno, void *buf, usize nbytes);

long
sys_bread(void)
{
    uint dev;
    u64 blockno;
    byte *buf;
    usize nbytes;

    if (argint(0, (int *)&dev) == -1) {
        cprintf("sys_bread - argint\n");
        return -1;
    }
    if (arglong(1, (long *)&blockno) == -1) {
        cprintf("sys_bread - arglong1\n");
        return -1;
    }
    if (arglong(3, (long *)&nbytes) == -1) {
        cprintf("sys_bread - arglong3\n");
        return -1;
    }
    if (argptr(2, (uintptr *)&buf, nbytes) == -1) {
        cprintf("sys_bread - argptr\n");
        return -1;
    }

    Buf *b = bread(dev, blockno);
    memmove(buf, b->data, nbytes);
    brelse(b);

    return 0;
}
