#include "types.h"

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

int sys_hello(SyscallFrame *);
int sys_goodbye(SyscallFrame *);

static int (*syscalls[])(SyscallFrame *) = {
    [SYS_HELLO] sys_hello,
    [SYS_GOODBYE] sys_goodbye,
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
arglong(SyscallFrame *f, int n)
{
    if (n > 5)
        panic("arglong");

    return f->args[n];
}

int
sys_hello(SyscallFrame *f)
{
    static int i = 0;
    cprintf("sys_hello(%d, %d, %d, %d, %d, %d): %d\n",
            arglong(f, 0), arglong(f, 1), arglong(f, 2),
            arglong(f, 3), arglong(f, 4), arglong(f, 5),
            i++);
    return 0;
}

int
sys_goodbye(SyscallFrame *f)
{
    static int i = 0;
    cprintf("sys_goodbye(%d, %d, %d, %d, %d, %d): %d\n",
            arglong(f, 0), arglong(f, 1), arglong(f, 2),
            arglong(f, 3), arglong(f, 4), arglong(f, 5),
            i++);
    return 0;
}

int
syscall(SyscallFrame *f)
{
    if (f->num > 0 && f->num < NELEM(syscalls) && syscalls[f->num]) {
        return syscalls[f->num](f);
    } else {
        cprintf("unknown syscall: %d\n", f->num);
        return -1;
    }
}
