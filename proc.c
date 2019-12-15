#include "u.h"

#include "defs.h"
#include "elf.h"
#include "lib.h"
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

Cpu *
mycpu(void)
{
    return cpu;
}

// should be called holding ptable.lock
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

#include "arm64.h"

void
scheduler(void)
{
    Proc *p;

    for (;;) {
        intr_on();

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
procbegin(void)
{
    unlock(&ptable.lock);

    // returns to the process entry point. See mkproc's first arg.
}

static Proc *
allocproc(void)
{
    Proc *p;

    lock(&ptable.lock);

    for (p = ptable.procs; p < &ptable.procs[NPROCS]; p++)
        if (p->state == UNUSED)
            goto found;

    unlock(&ptable.lock);
    return nil;

found:
    p->state = EMBRYO;
    p->pid = nextpid++;

    unlock(&ptable.lock);

    if ((p->kstack = kalloc()) == nil) {
        p->state = UNUSED;
        return nil;
    }

    initkstack(p);

    return p;
}

static void
freeproc(Proc *p)
{
    lock(&ptable.lock);

    if (p->kstack) {
        kfree(p->kstack);
        p->kstack = nil;
    }

    if (p->pgmap) {
        freeuvm(p->pgmap);
        p->pgmap = nil;
    }

    p->state = UNUSED;

    // TODO: close file references

    unlock(&ptable.lock);
}

void
mkproc(byte *data)
{
    Proc *p;
    ElfHeader *elf;
    ElfProgHeader *ph, *eph;

    p = allocproc();
    if (p == nil)
        panic("mkproc - allocproc");

    p->pgmap = allocpgmap();
    if (p->pgmap == nil) {
        panic("mkproc - pgmap");
    }

    memzero(p->errstr, ERRMAX);
    p->nextfd = 0;
    p->sz = 0;

    elf = (ElfHeader *)data;

    if (elf->magic != ELF_MAGIC)
        panic("mkproc - elf magic");

    ph = (ElfProgHeader *)(data + elf->phoff);
    eph = ph + elf->phnum;

    for (; ph < eph; ph++) {
        if (ph->type != ELF_PROG_LOAD)
            continue;
        if (ph->filesz > ph->memsz)
            panic("mkproc - filesz");
        if (ph->vaddr % PGSIZE != 0)
            panic("mkproc - align");
        if ((p->sz = allocuvm(p->pgmap, p->sz, p->sz + ph->memsz)) == 0)
            panic("mkproc - allocuvm");

        loaduvm(p->pgmap, (void *)ph->vaddr, data + ph->offset, ph->filesz);
    }

    // allocate stack. lower page is a guard
    p->sz = (usize)pgceil((void *)p->sz);
    if ((p->sz = allocuvm(p->pgmap, p->sz, p->sz + 2*PGSIZE)) == 0)
        panic("mkproc - alloc stack");
    clearpteu(p->pgmap, (void *)(p->sz - 2*PGSIZE));

    initstack(p, elf->entry);

    lock(&ptable.lock);
    p->state = READY;
    unlock(&ptable.lock);
}

void
sleep(void *chan, SpinLock *l)
{
    if (l != &ptable.lock) {
        lock(&ptable.lock);
        unlock(l);
    }

    proc->chan = chan;
    proc->state = WAITING;

    sched();

    proc->chan = nil;

    if (l != &ptable.lock) {
        unlock(&ptable.lock);
        lock(l);
    }
}

void
wakeup(void *chan)
{
    Proc *p;

    lock(&ptable.lock);

    for (p = ptable.procs; p < ptable.procs+NPROCS; p++) {
        if (p->state == WAITING && p->chan == chan) {
            p->state = READY;
        }
    }

    unlock(&ptable.lock);
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
    intr_off();
    cprintf("cpu%d: panic: ", cpu->id);

    cvprintf(fmt, ap);

    cputc('\n');

    for (;;)
        halt();
}

int
rfork(int flags)
{
    Proc *newp;

    if (!(flags & RFPROC))
        return 0;

    newp = allocproc();
    if (newp == nil) {
        // todo errstr
        return -1;
    }

    newp->pgmap = copyuvm(proc->pgmap, proc->sz);
    if (newp->pgmap == nil) {
        freeproc(newp);
        return -1;
    }

    newp->sz = proc->sz;
    newp->parent = proc;

    if (!(flags & RFFDG))
        panic("rfork - cannot share fd table yet");
    else
        copyfds(proc, newp); // also copy nextfd here

    archrfork(newp, proc);

    lock(&ptable.lock);
    newp->state = READY;
    unlock(&ptable.lock);

    return newp->pid;
}

long
sys_rfork(void)
{
    int flags;

    if (argint(0, &flags) < 0) {
        // todo errstr
        return -1;
    }

    return rfork(flags);
}

void
schedinit(void)
{
    initlock(&ptable.lock, "ptable");

    cpu = &cpus[0];
    cpu->id = 0;
    cpu->noff = 0;
}
