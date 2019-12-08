#include "u.h"

#include "defs.h"
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
    void *rest; // the rest of physical memory that's not yet managed by the free list
    void *end; // the end of physical memory
} kmem;


void *
p2v(uintptr paddr)
{
    return (void *)(paddr + KERNBASE);
}

uintptr
v2p(void *vaddr)
{
    return (uintptr)vaddr - KERNBASE;
}

void *
pgfloor(void *addr)
{
    return (void *)((uintptr)addr & ~(PGSIZE-1));
}

void *
pgceil(void *addr)
{
    uintptr a = (uintptr)addr;
    return (void *)((a+PGSIZE-1) & ~(PGSIZE-1));
}

static void
kfree0(void *a, int uselock)
{
    FreeList *page;
    char *p = (char *)a;

    if ((u64)p % PGSIZE || p < end || v2p(p) >= PHYSTOP)
        panic("kfree");

    // invalidate old data
    memset(p, 1, PGSIZE);

    if (uselock)
        lock(&kmem.lock);

    page = (FreeList *)p;
    page->next = kmem.freelist;
    kmem.freelist = page;

    if (uselock)
        unlock(&kmem.lock);
}

void
kfree(void *a)
{
    kfree0(a, kmem.uselock);
}

static void
freemore(void)
{
    if (kmem.rest >= kmem.end)
        return;

    kfree0(kmem.rest, 0);
    kmem.rest += PGSIZE;
}

// Allocate a page. Returns zero on failure.
void *
kalloc(void)
{
    FreeList *page;

    if (kmem.uselock)
        lock(&kmem.lock);

    if (!kmem.freelist)
        freemore();

    if (!kmem.freelist)
        return nil;

    page = kmem.freelist;
    kmem.freelist = page->next;

    if (kmem.uselock)
        unlock(&kmem.lock);

    return (void *)page;
}


void
initmem1(void *start, void *end)
{
    initlock(&kmem.lock, "kmem");
    kmem.freelist = nil;

    // interrupts aren't set up yet, so we can't call lock/unlock
    kmem.uselock = 0;

    kmem.rest = pgceil(start);
    kmem.end = pgceil(end);
}

void
initmem2(void *end)
{
    kmem.end = pgceil(end);
    kmem.uselock = 1;
}
