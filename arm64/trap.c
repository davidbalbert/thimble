#include "u.h"

#include "arm64.h"
#include "defs.h"

void
trap(u64 type, u64 esr, u64 elr, u64 spsr, unsigned long far)
{
    // print out interruption type
    switch(type) {
        case 0: cprintf("Synchronous"); break;
        case 1: cprintf("IRQ"); break;
        case 2: cprintf("FIQ"); break;
        case 3: cprintf("SError"); break;
    }
    cprintf(": ");
    // decode exception type (some, not all. See ARM DDI0487B_b chapter D10.2.28)
    switch(esr>>26) {
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
    if(esr>>26==0b100100 || esr>>26==0b100101) {
        cprintf(", ");
        switch((esr>>2)&0x3) {
            case 0: cprintf("Address size fault"); break;
            case 1: cprintf("Translation fault"); break;
            case 2: cprintf("Access flag fault"); break;
            case 3: cprintf("Permission fault"); break;
        }
        switch(esr&0x3) {
            case 0: cprintf(" at level 0"); break;
            case 1: cprintf(" at level 1"); break;
            case 2: cprintf(" at level 2"); break;
            case 3: cprintf(" at level 3"); break;
        }
    }
    // dump registers
    cprintf(":\n  ESR_EL1 0x%x", esr);
    cprintf(" ELR_EL1 0x%x", elr);
    cprintf("\n SPSR_EL1 0x%x", spsr);
    cprintf(" FAR_EL1 0x%x", far);
    cprintf("\n");
    // no return from exception for now
    while(1);
}

void
trapinit(void)
{

}
