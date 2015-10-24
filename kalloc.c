#include "types.h"

#include "common.h"
#include "lock.h"
#include "mem.h"

struct FreeList {
    struct FreeList *next;
};
typedef struct FreeList FreeList;

static struct {
    SpinLock lock;
    FreeList *freelist;
    int uselock;
} kmem;


void *
p2v(ulong paddr)
{
    return (void *)(paddr + KERNBASE);
}

ulong
v2p(void *vaddr)
{
    return (ulong)vaddr - KERNBASE;
}

void *
pgfloor(void *addr)
{
    return (void *)((ulong)addr & ~(PGSIZE-1));
}

void *
pgceil(void *addr)
{
    ulong a = (ulong)addr;
    return (void *)((a+PGSIZE-1) & ~(PGSIZE-1));
}

// Allocate a page. Returns zero on failure.
void *
kalloc(void)
{
    FreeList *page;

    if (kmem.uselock)
        lock(&kmem.lock);

    if (!kmem.freelist)
        return 0;

    page = kmem.freelist;
    kmem.freelist = page->next;

    if (kmem.uselock)
        unlock(&kmem.lock);

    return (void *)page;
}

void
kfree(void *a)
{
    FreeList *page;
    char *p = (char *)a;

    if ((ulong)p % PGSIZE || p < end || v2p(p) >= PHYSTOP)
        panic("kfree");

    // invalidate old data
    memset(p, 1, PGSIZE);

    if (kmem.uselock)
        lock(&kmem.lock);

    page = (FreeList *)p;
    page->next = kmem.freelist;
    kmem.freelist = page;

    if (kmem.uselock)
        unlock(&kmem.lock);
}

static void
freerange(void *start, void *end)
{
    char *p;

    start = pgceil(start);
    end = pgceil(end);

    for (p = start; p < (char *)end; p += PGSIZE)
        kfree(p);
}

void
initmem1(void *start, void *end)
{
    initlock(&kmem.lock);
    kmem.freelist = 0;

    // interrupts aren't set up yet, so we can't call lock/unlock
    kmem.uselock = 0;

    freerange(start, end);
}
