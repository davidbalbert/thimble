#include "u.h"

#include "archdefs.h"
#include "arm64.h"
#include "defs.h"
#include "irq.h"

#define ESR_SYSCALL 0b010101

extern u8 vectors[];

static void
othertrap(TrapFrame *tf)
{
    // print out interruption type
    switch(tf->type) {
        case 0: cprintf("Synchronous"); break;
        case 1: cprintf("IRQ"); break;
        case 2: cprintf("FIQ"); break;
        case 3: cprintf("SError"); break;
    }
    cprintf(": ");
    // decode exception type (some, not all. See ARM DDI0487B_b chapter D10.2.28)
    switch(tf->esr>>26) {
        case 0b000000: cprintf("Unknown"); break;
        case 0b000001: cprintf("Trapped WFI/WFE"); break;
        case 0b001110: cprintf("Illegal execution"); break;
        case 0b010101: cprintf("System call"); break;
        case 0b100000: cprintf("Instruction abort, lower EL"); break;
        case 0b100001: cprintf("Instruction abort, same EL"); break;
        case 0b100010: cprintf("Instruction alignment fault"); break;
        case 0b100100: cprintf("Data abort, lower EL"); break;
        case 0b100101: cprintf("Data abort, same EL"); break;
        case 0b100110: cprintf("Stack alignment fault"); break;
        case 0b101100: cprintf("Floating point"); break;
        default: cprintf("Unknown"); break;
    }
    // decode data abort cause
    if(tf->esr>>26==0b100100 || tf->esr>>26==0b100101) {
        cprintf(", ");
        switch((tf->esr>>2)&0x3) {
            case 0: cprintf("Address size fault"); break;
            case 1: cprintf("Translation fault"); break;
            case 2: cprintf("Access flag fault"); break;
            case 3: cprintf("Permission fault"); break;
        }
        switch(tf->esr&0x3) {
            case 0: cprintf(" at level 0"); break;
            case 1: cprintf(" at level 1"); break;
            case 2: cprintf(" at level 2"); break;
            case 3: cprintf(" at level 3"); break;
        }
    }
    // dump registers
    cprintf(":\n  ESR_EL1 0x%x", tf->esr);
    cprintf(" ELR_EL1 0x%x", tf->elr);
    cprintf("\n SPSR_EL1 0x%x", tf->spsr);
    cprintf(" FAR_EL1 0x%x", tf->far);
    cprintf("\n");

    for (;;) {
        halt();
    }
}

void
trap(TrapFrame *tf)
{
    if (tf->type == 0 && (tf->esr >> 26) == ESR_SYSCALL) {
        syscall(tf);
        return;
    }

    if (tf->type != 1) {
        othertrap(tf); // doesn't return
    }

    u32 trapno = readirq();

    switch (trapno) {
        case IRQ_TIMER:
            handletimer();
            break;
        case IRQ_UART:
            handleuart();
            break;
        default:
            cprintf("trap: %u\n", trapno);
            //dumpregs(tf);
            for (;;) {
                halt();
            }
    }
}

void
trapinit(void)
{
    st_vbar_el1(vectors);
}
