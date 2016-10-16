#include "u.h"

#include "defs.h"
#include "mem.h"
#include "proc.h"
#include "syscall.h"
#include "x86.h"

struct SyscallFrame {
    ulong num;
    ulong args[6];
};
typedef struct SyscallFrame SyscallFrame;

long sys_hello(SyscallFrame *);
long sys_goodbye(SyscallFrame *);
long sys_print(SyscallFrame *);
long sys_printlong(SyscallFrame *);

long sys_open(SyscallFrame *);
long sys_close(SyscallFrame *);
long sys_read(SyscallFrame *);

static long (*syscalls[])(SyscallFrame *) = {
    [SYS_HELLO] sys_hello,
    [SYS_GOODBYE] sys_goodbye,
    [SYS_PRINT] sys_print,
    [SYS_PRINTLONG] sys_printlong,
    [SYS_OPEN] sys_open,
    [SYS_CLOSE] sys_close,
    [SYS_READ] sys_read,
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
sys_hello(SyscallFrame *f)
{
    static int i = 0;

    long a, b, c, d, e, g;
    int fail =
        arglong(f, 0, &a) ||
        arglong(f, 1, &b) ||
        arglong(f, 2, &c) ||
        arglong(f, 3, &d) ||
        arglong(f, 4, &e) ||
        arglong(f, 5, &g);

    if (fail)
        return -1;

    cprintf("sys_hello(%d, %d, %d, %d, %d, %d): %d\n",
            a, b, c, d, e, g, i++);
    return 0;
}

long
sys_goodbye(SyscallFrame *f)
{
    static int i = 0;

    long a, b, c, d, e, g;
    int fail =
        arglong(f, 0, &a) ||
        arglong(f, 1, &b) ||
        arglong(f, 2, &c) ||
        arglong(f, 3, &d) ||
        arglong(f, 4, &e) ||
        arglong(f, 5, &g);

    if (fail)
        return -1;

    cprintf("sys_goodbye(%d, %d, %d, %d, %d, %d): %d\n",
            a, b, c, d, e, g, i++);
    return 0;
}

long
sys_print(SyscallFrame *f)
{
    char *s;
    if (argstr(f, 0, &s) < 0)
        return -1;

    cprintf("%s", s);

    return 0;
}

long
sys_printlong(SyscallFrame *f)
{
    long l;

    if (arglong(f, 0, &l) < 0)
        return -1;

    cprintf("%l\n", l);

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
