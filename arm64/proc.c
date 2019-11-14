#include "u.h"

#include "arm64.h"
#include "asm.h"
#include "defs.h"
#include "mem.h"
#include "proc.h"

void trapret(void);

// build the kernel stack for a new process
void
initkstack(Proc *p)
{
    byte *sp = p->kstack + KSTACKSIZE;

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
}

// build the user stack for a new process
void
initstack(Proc *p, u64 entry)
{
    p->tf->sp_el0 = p->sz; // user virtual address of EL0 stack base
    p->tf->elr = entry;
}

// architecture specific fork logic
void
archrfork(Proc *new, Proc *old)
{
    *new->tf = *old->tf;
    new->tf->x0 = 0; // return 0 in child
}
