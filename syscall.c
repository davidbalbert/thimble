#include "types.h"

#include "common.h"
#include "mem.h"

// Be careful. This is called in kernel mode but on the user's
// stack. We use it to find out where the kernel stack is that we
// should switch to.
uchar *
kstacktop(void)
{
    if (proc = 0)
        panic("kstacktop");

    return proc->kstack + KSTACKSIZE;
}

int
syscall(int num)
{
    cprintf("syscall(%d)\n", num);
    return 0;
}
