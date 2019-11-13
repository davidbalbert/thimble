#include "u.h"

#include "defs.h"
#include "mem.h"
#include "proc.h"
#include "syscall.h"
#include "systable.h"

// Be careful. This is called in kernel mode but on the user's
// stack. We use it to find out where the kernel stack is that we
// should switch to.
uchar *
kstacktop(void)
{
    if (proc == nil)
        panic("kstacktop");

    return proc->kstack + KSTACKSIZE;
}

// Fetch the nth syscall argument. N starts at 0.
int
arglong(int n, long *lp)
{
    if (n < 0 || n > 5)
        panic("arglong");

    *lp = proc->sf->args[n];
    return 0;
}

void
syscall(SyscallFrame *f)
{
    if (f->num > 0 && f->num < nelem(syscalls) && syscalls[f->num]) {
        proc->sf = f;
        f->rax = syscalls[f->num]();
    } else {
        cprintf("unknown syscall: %d\n", f->num);
        f->rax = -1;
    }
}
