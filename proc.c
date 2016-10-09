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
mkproc(uchar *data, usize memsz)
{
    Proc *p;
    uchar *ksp, *usp;

    if (ptable.n >= NPROCS)
        panic("mkproc - nprocs");

    p = &ptable.procs[ptable.n++];

    p->kstack = kalloc();
    if (p->kstack == nil)
        panic("mkproc - kstack");

    p->state = READY;

    ksp = p->kstack + KSTACKSIZE;

    p->pgmap = setupkvm();
    if (p->pgmap == nil)
        panic("mkproc - pgmap");

    p->sz = 0;
    if ((p->sz = allocuvm(p->pgmap, p->sz, memsz)) == 0)
        panic("mkproc - alloc text");

    loaduvm(p->pgmap, (void *)0, data, memsz);

    // allocate stack. lower page is a guard
    if ((p->sz = allocuvm(p->pgmap, p->sz, 2*PGSIZE)) == 0)
        panic("mkproc - alloc stack");
    clearpteu(p->pgmap, (void *)(p->sz - 2*PGSIZE));

    // get user stack pointer)
    usp = uva2ka(p->pgmap, (void *)p->sz);

    // fake return address
    usp -= 8;
    *(ulong *)usp = (ulong)-1;

    // This has to mirror the stack structure in
    // syscallasm.S.

    // %r12 and %r13 are used as temporary storage
    usp -= 16;

    // rflags
    usp -= 8;
    *(ulong *)usp = FL_IF;

    // entry point
    usp -= 8;
    *(ulong *)usp = (ulong)0; // text is loaded at zero

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
start(uchar *data, usize size)
{
    lock(&ptable.lock);
    mkproc(data, size);
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
            switchkvm();

            proc = nil;
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
