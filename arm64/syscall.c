#include "u.h"

#include "archdefs.h"
#include "arm64.h"
#include "defs.h"
#include "proc.h"
#include "syscall.h"

long sys_printint(void);

static long (*syscalls[])(void) = {
    [SYS_PRINTINT] sys_printint,
};

long
sys_printint(void)
{
    long l;

    arglong(0, &l);
    cprintf("%l\n", l);

    return 0;
}

// Fetch the nth syscall argument. N starts at 0.
long
arglong(int n, long *lp)
{
    if (n < 0 || n > 5)
        panic("arglong");

    *lp = *(long *)(&proc->tf->x0 + n);

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
