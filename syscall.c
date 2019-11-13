#include "u.h"

#include "defs.h"
#include "file.h"
#include "proc.h"

int
argint(int n, int *ip)
{
    long l;

    if (n < 0 || n > 5)
        panic("argint");

    if (arglong(n, &l) < 0)
        return -1;

    *ip = (int)l;
    return 0;
}

int
argptr(int n, uintptr *p, usize size)
{
    long l;

    if (n < 0 || n > 5)
        panic("argptr");

    if (arglong(n, &l) < 0)
        return -1;
    if ((uintptr)l >= proc->sz || (uintptr)l+size > proc->sz)
        return -1;

    *p = (uintptr)l;

    return 0;
}

// returns length of string, not including null
long
argstr(int n, char **pp)
{
    long l;
    uintptr addr;
    char *p;

    if (n < 0 || n > 5)
        panic("argstr");

    if (arglong(n, &l) < 0)
        return -1;

    addr = (uintptr)l;

    if (addr >= proc->sz)
        return -1;

    *pp = (char *)addr;
    for (p = (char *)addr; p < (char *)proc->sz; p++)
        if (*p == 0)
            return p - *pp;

    return -1;
}

int
argfd(int n, int *fd)
{
    int i;

    if (n < 0 || n > 5)
        panic("argfd");

    if (argint(n, &i) < 0)
        return -1;

    if (i >= proc->nextfd || proc->files[i] == nil) {
        // todo errstr
        return -1;
    }

    if (proc->files[i]->ref < 1)
        panic("argfd - proc->files[fd] references unallocated file");

    *fd = i;

    return 0;
}
