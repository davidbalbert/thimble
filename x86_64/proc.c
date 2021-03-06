#include "u.h"

#include "cpu.h"
#include "archdefs.h"
#include "defs.h"
#include "mem.h"
#include "proc.h"
#include "x86.h"

void sysret(void);

// build the kernel stack for a new process
void
initkstack(Proc *p)
{
    byte *sp = p->kstack + KSTACKSIZE;

    // This has to match the kernel stack structure in
    // syscallasm.S.

    sp -= sizeof(SyscallFrame);
    p->sf = (SyscallFrame *)sp;
    memzero(p->sf, sizeof(SyscallFrame));

    // Procbegin returns to sysret
    sp -= 8;
    *(u64 *)sp = (u64)sysret;

    sp -= sizeof(Registers);

    p->regs = (Registers *)sp;
    memzero(p->regs, sizeof(Registers));

    // Our first call to swtch will return to procbegin
    p->regs->rip = (u64)procbegin;

}

void
initstack(Proc *p, u64 entry)
{
    byte *usp;     // user virtual address of user stack pointer
    byte *ustack;  // kernel virtual address of user stack pointer

    // get user stack pointer
    usp = (byte *)p->sz;

    // p->sz isn't mapped in (0 to p->sz - 1 is), so we can't just
    // ask uva2ka for the kernel address of p->sz
    ustack = uva2ka(p->pgmap, (void *)(p->sz - PGSIZE)) + PGSIZE;

    // fake return address
    ustack -= 8;
    usp -= 8;
    *(u64 *)ustack = (u64)-1;

    // This has to mirror the user stack structure in
    // syscallasm.S.

    // %r12 and %r13 are used as temporary storage
    ustack -= 16;
    usp -= 16;

    // rflags
    ustack -= 8;
    usp -= 8;
    *(u64 *)ustack = FL_IF;

    // entry point
    ustack -= 8;
    usp -= 8;
    *(u64 *)ustack = entry; // text is loaded at zero

    p->sf->rsp = (u64)usp;
}

void
archrfork(Proc *new, Proc *old)
{
    new->sf->rax = 0; // return 0 in child
    new->sf->rsp = old->sf->rsp;
}
