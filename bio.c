#include "u.h"

#include "archdefs.h"
#include "defs.h"
#include "lock.h"

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
        initlock(&b->lock, "buf");
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
            lock(&b->lock);
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
            lock(&b->lock);
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
    unlock(&b->lock);

    lock(&bcache.lock);
    b->refcnt--;
    unlock(&bcache.lock);
}

void bwrite(Buf *b)
{
    panic("not implemented");
}
