#include "u.h"

#include "cpu.h"
#include "defs.h"
#include "elf.h"
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
mkproc(uchar *data)
{
    Proc *p;
    uchar *ksp;     // kernal virtual address of kernal stack pointer
    uchar *usp;     // kernel virtual address of user stack pointer
    uchar *ustack;  // user virtual address of user stack pointer
    ElfHeader *elf;
    ElfProgHeader *ph, *eph;

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
    ustack = (uchar *)p->sz;

    // p->sz isn't mapped in (0 to p->sz - 1 is), so we can't just
    // ask uva2ka for the kernel address of p->sz
    usp = uva2ka(p->pgmap, (void *)(p->sz - PGSIZE)) + PGSIZE;

    // fake return address
    usp -= 8;
    ustack -= 8;
    *(ulong *)usp = (ulong)-1;

    // This has to mirror the stack structure in
    // syscallasm.S.

    // %r12 and %r13 are used as temporary storage
    usp -= 16;
    ustack -= 16;

    // rflags
    usp -= 8;
    ustack -= 8;
    *(ulong *)usp = FL_IF;

    // entry point
    usp -= 8;
    ustack -= 8;
    *(ulong *)usp = elf->entry; // text is loaded at zero

    // user stack
    ksp -= 8;
    *(ulong *)ksp = (ulong)ustack;

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

void
start(uchar *data)
{
    lock(&ptable.lock);
    mkproc(data);
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
