#include "u.h"

#include "archdefs.h"
#include "arm64.h"
#include "defs.h"
#include "file.h"
#include "proc.h"
#include "syscall.h"

long sys_open(void);
long sys_close(void);
long sys_read(void);
long sys_write(void);
long sys_rfork(void);

static long (*syscalls[])(void) = {
    [SYS_OPEN] sys_open,
    [SYS_CLOSE] sys_close,
    [SYS_READ] sys_read,
    [SYS_WRITE] sys_write,
    [SYS_RFORK] sys_rfork,
};

// Fetch the nth syscall argument. N starts at 0.
int
arglong(int n, long *lp)
{
    if (n < 0 || n > 5)
        panic("arglong");

    *lp = *(long *)(&proc->tf->x0 + n);

    return 0;
}

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

void
syscall(TrapFrame *tf)
{
    int num = tf->x8;

    if (num > 0 && num < nelem(syscalls) && syscalls[num]) {
        proc->tf = tf;
        tf->x0 = syscalls[num]();
    } else {
        cprintf("unknown syscall: %d\n", num);
        tf->x0 = -1;
    }
}
