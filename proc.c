#include "types.h"

#include "defs.h"
#include "cpu.h"
#include "lock.h"
#include "mem.h"
#include "proc.h"
#include "x86.h"

#define NPROCS 1024

static struct {
    SpinLock lock;
    int n;
    Proc procs[NPROCS];
} ptable;

#define MAXCPU 8

static Cpu cpus[MAXCPU];

Cpu  *cpu;  // current cpu
Proc *proc; // current process

static void
procbegin(void)
{
    unlock(&ptable.lock);

    // returns to sysret, which sysrets to the process entry point
}

void sysret(void);

static void
mkproc(void (*f)(void))
{
    Proc *p;
    uchar *ksp;
    uchar *usp;

    if (ptable.n >= NPROCS)
        panic("mkproc - nprocs");

    p = &ptable.procs[ptable.n++];

    p->kstack = kalloc();
    if (p->kstack == 0)
        panic("mkproc - kstack");

    p->ustack = kalloc();
    if (p->ustack == 0)
        panic("mkproc - ustack");

    p->state = READY;

    ksp = p->kstack + KSTACKSIZE;
    usp = p->ustack + USTACKSIZE;

    // This has to mirror the stack structure in
    // syscallasm.S.

    // %r12 and %r13 are used as temporary storage
    usp -= 16;

    // rflags
    usp -= 8;
    *(ulong *)usp = FL_IF;

    // entry point
    usp -= 8;
    *(ulong *)usp = (ulong)f;

    // user stack
    ksp -= 8;
    *(ulong *)ksp = (ulong)usp;

    // SyscallFrame (syscall num and 6 args)
    ksp -= 7*8;

    // Procbegin returns to sysret
    ksp -= 8;
    *(ulong *)ksp = (ulong)sysret;

    ksp -= sizeof(Registers);

    p->regs = (Registers *)ksp;
    memzero(p->regs, sizeof(Registers));

    // Our first call to swtch will return to procbegin
    p->regs->rip = (ulong)procbegin;
}

void
panic(char *s)
{
    cli();
    cprintf("cpu%d: panic: %s\n", cpu->id, s);

    for (;;)
        hlt();
}

void
start(void (*f)(void))
{
    lock(&ptable.lock);
    mkproc(f);
    unlock(&ptable.lock);
}

static void
sched(void)
{
    int intena;

    intena = cpu->intena;
    swtch(&proc->regs, cpu->scheduler);
    cpu->intena = intena;
}

void
yield(void)
{
    lock(&ptable.lock);
    proc->state = READY;
    sched();
    unlock(&ptable.lock);
}

void
scheduler(void)
{
    Proc *p;

    sti();

    for (;;) {
        lock(&ptable.lock);
        for (p = ptable.procs; p < &ptable.procs[NPROCS]; p++) {
            if (p->state != READY)
                continue;

            p->state = RUNNING;
            proc = p;

            switchuvm(proc);
            swtch(&cpu->scheduler, p->regs);

            proc = 0;
        }
        unlock(&ptable.lock);
    }

}

void
schedinit(void)
{
    initlock(&ptable.lock);

    cpu = &cpus[0];
    cpu->id = 0;
    cpu->ncli = 0;
}
