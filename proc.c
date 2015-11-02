#include "types.h"

#include "common.h"
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

    // In "user space," interrupts are always on. There are also no
    // kernel locks held, so enabling interrupts should not be a
    // problem here. This will eventually change to something else
    // when we actually have user space programs.
    sti();

    // returns to the process entry point. See mkproc's first arg.
}

static void
mkproc(void (*f)(void))
{
    Proc *p;
    uchar *sp;

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

    sp = p->kstack + PGSIZE;
    sp -= 8;

    // Procbegin returns to f, our entry point
    *(ulong *)sp = (ulong)f;

    sp -= sizeof(Registers);

    p->regs = (Registers *)sp;
    memzero(p->regs, sizeof(Registers));

    // Our first call to swtch will return to procbegin
    p->regs->rip = (ulong)procbegin;
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
