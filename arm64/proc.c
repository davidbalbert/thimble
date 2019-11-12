#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "lock.h"
#include "mem.h"
#include "proc.h"

#define NPROCS 1024

static struct {
    SpinLock lock;
    Proc procs[NPROCS];
} ptable;

int nextpid = 1;

#define MAXCPU 8

static Cpu cpus[MAXCPU];

Cpu  *cpu;  // current cpu
Proc *proc; // current proc

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

            swtch(&cpu->scheduler, p->regs);

            proc = nil;
        }
        unlock(&ptable.lock);
    }
}

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

void
mkproc(void (*f)(void))
{
    Proc *p;
    u8 *sp;

    lock(&ptable.lock);

    for (p = ptable.procs; p < &ptable.procs[NPROCS]; p++)
        if (p->state == UNUSED)
            goto found;

    unlock(&ptable.lock);
    return;

found:
    p->state = EMBRYO;
    p->pid = nextpid++;

    unlock(&ptable.lock);

    if ((p->kstack = kalloc()) == nil) {
        p->state = UNUSED;
        return;
    }

    sp = p->kstack + KSTACKSIZE;

    // LR (x30)
    // FP (x29) <- SP
    sp -= 16; // LR and FP
    // procbegin returns to f
    *(u64 *)(sp + 8) = (u64)f;
    *(u64 *)sp = 0;

    sp -= sizeof(Registers);

    p->regs = (Registers *)sp;
    memzero(p->regs, sizeof(Registers));

    // Swtch returns to procbegin. Becase the first two instructions (4 bytes
    // per instruction) push LR and FP onto the stack, we want to skip them.
    p->regs->x30 = (u64)procbegin+8;
    p->state = READY;
}

void
start(void (*f)(void))
{
    mkproc(f);
}

void
panic(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vpanic(fmt, ap);
    va_end(ap);
}

void
vpanic(char *fmt, va_list ap)
{
    cli();
    cprintf("cpu%d: panic: ", cpu->id);

    cvprintf(fmt, ap);

    cputc('\n');

    for (;;)
        halt();
}

void
schedinit(void)
{
    initlock(&ptable.lock);

    cpu = &cpus[0];
    cpu->id = 0;
    cpu->ncli = 0;
}
