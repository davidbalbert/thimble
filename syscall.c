#include "types.h"

#include "defs.h"
#include "mem.h"
#include "proc.h"
#include "syscall.h"

int sys_hello(void);
int sys_goodbye(void);

static int (*syscalls[])(void) = {
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

int
sys_hello(void)
{
    static int i = 0;
    cprintf("sys_hello: %d\n", i++);
    return 0;
}

int
sys_goodbye(void)
{
    static int i = 0;
    cprintf("sys_goodbye: %d\n", i++);
    return 0;
}

int
syscall(int num)
{
    if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        return syscalls[num]();
    } else {
        cprintf("unknown syscall: %d\n", num);
        return -1;
    }
}
