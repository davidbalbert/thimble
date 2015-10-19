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

#define STACKSIZE PGSIZE
static uchar stacks[NPROCS][STACKSIZE] __attribute__((aligned(16)));


#define MAXCPU 8

static Cpu cpus[MAXCPU];

Cpu  *cpu;  // current cpu
Proc *proc; // current process

static void
procbegin(void)
{
    unlock(&ptable.lock);
    // returns to the process entry point. See mkproc's first arg.
}

static void
mkproc(void (*f)(void))
{
    Proc *p;
    uchar *sp;

    if (ptable.n >= NPROCS)
        panic("mkproc");

    p = &ptable.procs[ptable.n];
    p->kstack = stacks[ptable.n++];
    p->state = READY;

    sp = p->kstack + STACKSIZE;
    sp -= 8;

    // Procbegin returns to f, our entry point
    *(ulong *)sp = (ulong)f;

    sp -= sizeof(Registers);

    p->regs = (Registers *)sp;
    // TODO?: memset p->regs

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

    for (;;) {
        lock(&ptable.lock);
        for (p = ptable.procs; p < &ptable.procs[NPROCS]; p++) {
            if (p->state != READY)
                continue;

            p->state = RUNNING;
            proc = p;

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
    cpu->ncli = 0;
}
