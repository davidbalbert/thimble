#include "u.h"

#include "archdefs.h"
#include "arm64.h"
#include "asm.h"
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

static void
procbegin(void)
{
    unlock(&ptable.lock);

    // returns to the process entry point. See mkproc's first arg.
}

void trapret(void);

static Proc *
allocproc(void)
{
    Proc *p;
    u8 *sp;

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

    sp = p->kstack + KSTACKSIZE;

    sp -= sizeof(TrapFrame);
    p->tf = (TrapFrame *)sp;
    memzero(sp, sizeof(TrapFrame));

    p->tf->spsr = SPSR_EL1_D | SPSR_EL1_A | SPSR_EL1_F | SPSR_EL1_M_EL0T;

    // LR (x30)
    // FP (x29) <- SP
    sp -= 16; // LR and FP
    // procbegin returns to trapret
    *(u64 *)(sp + 8) = (u64)trapret;
    *(u64 *)sp = 0;

    sp -= sizeof(Registers);

    p->regs = (Registers *)sp;
    memzero(p->regs, sizeof(Registers));

    // Swtch returns to procbegin. Skip the first two instructions (4 bytes per
    // instruction) that push LR and FP onto the stack. We've already done it.
    p->regs->x30 = (u64)procbegin+8;

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
mkproc(uchar *data)
{
    Proc *p;
    uchar *usp;     // user virtual address of user stack pointer
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

    // get user stack pointer
    usp = (uchar *)p->sz;

    p->tf->sp_el0 = (u64)usp;
    p->tf->elr = elf->entry;

    lock(&ptable.lock);
    p->state = READY;
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
    cli();
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

    *newp->tf = *proc->tf;
    newp->tf->x0 = 0; // return 0 in child

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
    initlock(&ptable.lock);

    cpu = &cpus[0];
    cpu->id = 0;
    cpu->ncli = 0;
}
