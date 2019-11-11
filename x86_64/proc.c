#include "u.h"

#include "cpu.h"
#include "archdefs.h"
#include "defs.h"
#include "elf.h"
#include "lib.h"
#include "lock.h"
#include "mem.h"
#include "proc.h"
#include "x86.h"

#define NPROCS 1024

static struct {
    SpinLock lock;
    Proc procs[NPROCS];
} ptable;

int nextpid = 1;

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

static Proc *
allocproc(void)
{
    Proc *p;
    uchar *sp;

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

    // This has to match the kernel stack structure in
    // syscallasm.S.

    sp -= sizeof(SyscallFrame);
    p->sf = (SyscallFrame *)sp;
    memzero(p->sf, sizeof(SyscallFrame));

    // Procbegin returns to sysret
    sp -= 8;
    *(ulong *)sp = (ulong)sysret;

    sp -= sizeof(Registers);

    p->regs = (Registers *)sp;
    memzero(p->regs, sizeof(Registers));

    // Our first call to swtch will return to procbegin
    p->regs->rip = (ulong)procbegin;

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
    uchar *ustack;  // kernel virtual address of user stack pointer
    ElfHeader *elf;
    ElfProgHeader *ph, *eph;

    p = allocproc();
    if (p == nil)
        panic("mkproc - allocproc");

    p->pgmap = setupkvm();
    if (p->pgmap == nil)
        panic("mkproc - pgmap");

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

    // p->sz isn't mapped in (0 to p->sz - 1 is), so we can't just
    // ask uva2ka for the kernel address of p->sz
    ustack = uva2ka(p->pgmap, (void *)(p->sz - PGSIZE)) + PGSIZE;

    // fake return address
    ustack -= 8;
    usp -= 8;
    *(ulong *)ustack = (ulong)-1;

    // This has to mirror the user stack structure in
    // syscallasm.S.

    // %r12 and %r13 are used as temporary storage
    ustack -= 16;
    usp -= 16;

    // rflags
    ustack -= 8;
    usp -= 8;
    *(ulong *)ustack = FL_IF;

    // entry point
    ustack -= 8;
    usp -= 8;
    *(ulong *)ustack = elf->entry; // text is loaded at zero

    p->sf->rsp = (ulong)usp;

    lock(&ptable.lock);
    p->state = READY;
    unlock(&ptable.lock);
}

void
panic(char *fmt, ...)
{
    va_list ap;

    cli();
    cprintf("cpu%d: panic: ", cpu->id);

    va_start(ap, fmt);
    cvprintf(fmt, ap);
    va_end(ap);

    cputc('\n');

    for (;;)
        hlt();
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

    newp->sf->rax = 0; // return 0 in child
    newp->sf->rsp = proc->sf->rsp;

    lock(&ptable.lock);
    newp->state = READY;
    unlock(&ptable.lock);

    return newp->pid;
}

long
sys_rfork(SyscallFrame *f)
{
    int flags;

    if (argint(f, 0, &flags) < 0) {
        // todo errstr
        return -1;
    }

    return rfork(flags);
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
