#include "u.h"

#include "arm64.h"
#include "defs.h"

struct TrapFrame {
    u64 spsr;
    u64 far;
    u64 esr;
    u64 elr;

    u64 x0;
    u64 x1;
    u64 x2;
    u64 x3;
    u64 x4;
    u64 x5;
    u64 x6;
    u64 x7;
    u64 x8;
    u64 x9;
    u64 x10;
    u64 x11;
    u64 x12;
    u64 x13;
    u64 x14;
    u64 x15;
    u64 x16;
    u64 x17;
    u64 x18;
    u64 x19;
    u64 x20;
    u64 x21;
    u64 x22;
    u64 x23;
    u64 x24;
    u64 x25;
    u64 x26;
    u64 x27;
    u64 x28;
    u64 x29;
    u64 x30;

    u64 type;
};
typedef struct TrapFrame TrapFrame;

extern u8 vectors[];

void
trap(TrapFrame *tf)
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
    // no return from exception for now
    while(1);
}

void
trapinit(void)
{
    st_vbar_el1(vectors);
}
