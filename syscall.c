#include "u.h"

#include "defs.h"
#include "file.h"
#include "mem.h"
#include "proc.h"
#include "syscall.h"
#include "x86.h"

struct SyscallFrame {
    ulong num;
    ulong args[6];
};
typedef struct SyscallFrame SyscallFrame;

long sys_open(SyscallFrame *);
long sys_close(SyscallFrame *);
long sys_read(SyscallFrame *);
long sys_write(SyscallFrame *);

static long (*syscalls[])(SyscallFrame *) = {
    [SYS_OPEN] sys_open,
    [SYS_CLOSE] sys_close,
    [SYS_READ] sys_read,
    [SYS_WRITE] sys_write,
};

// Be careful. This is called in kernel mode but on the user's
// stack. We use it to find out where the kernel stack is that we
// should switch to.
uchar *
kstacktop(void)
{
    if (proc == 0)
        panic("kstacktop");

    return proc->kstack + KSTACKSIZE;
}

// Fetch the nth syscall argument. N starts at 0.
long
arglong(SyscallFrame *f, int n, long *ip)
{
    if (n > 5)
        panic("arglong");

    *ip = f->args[n];
    return 0;
}

long
argptr(SyscallFrame *f, int n, uintptr *p, usize size)
{
    long l;

    if (n > 5)
        panic("argptr");

    if (arglong(f, n, &l) < 0)
        return -1;
    if ((uintptr)l >= proc->sz || (uintptr)l+size > proc->sz)
        return -1;

    *p = (uintptr)l;

    return 0;
}

// returns length of string, not including null
long
argstr(SyscallFrame *f, int n, char **pp)
{
    long l;
    uintptr addr;
    char *p;

    if (n > 5)
        panic("argstr");

    arglong(f, n, &l);
    addr = (uintptr)l;

    if (addr >= proc->sz)
        return -1;

    *pp = (char *)addr;
    for (p = (char *)addr; p < (char *)proc->sz; p++)
        if (*p == 0)
            return p - *pp;

    return -1;
}

long
argfd(SyscallFrame *f, int n, int *fd)
{
    long l;
    int i;


    if (n > 5)
        panic("argstr");

    if (arglong(f, n, &l) < 0)
        return -1;
    i = (int)l;

    if (i >= proc->nextfd || proc->files[i] == nil) {
        // todo errstr
        return -1;
    }

    if (proc->files[i]->ref < 1)
        panic("argfd - proc->files[fd] references unallocated file");

    *fd = i;

    return 0;
}

long
syscall(SyscallFrame *f)
{
    if (f->num > 0 && f->num < nelem(syscalls) && syscalls[f->num]) {
        return syscalls[f->num](f);
    } else {
        cprintf("unknown syscall: %d\n", f->num);
        return -1;
    }
}
