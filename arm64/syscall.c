#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "proc.h"
#include "syscall.h"

long sys_printhello(void);

static long (*syscalls[])(void) = {
    [SYS_PRINTHELLO] sys_printhello,
};

long
sys_printhello(void)
{
    cprintf("Hello!\n");

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
