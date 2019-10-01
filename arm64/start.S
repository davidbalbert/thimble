#include "asm.h"

.global _start
_start:
    mrs x0, CurrentEL
    lsr x0, x0, #2 // shift EL down
    and x0, x0, #3 // clear other bits

    cmp x0, #3
    bne el1

    // Maybe we can get rid of SCR_EL3_HCE
    mov x2, #(SCR_EL3_RW | SCR_EL3_HCE | SCR_EL3_SMD | RES1 << 5 | RES1 << 4 | SCR_EL3_NS)
    msr SCR_EL3, x2

    mov x2, #(SPSR_EL3_D | SPSR_EL3_A | SPSR_EL3_I | SPSR_EL3_F | SPSR_EL3_M_EL2H)
    msr SPSR_EL3, x2

    adr x2, el2
    msr ELR_EL3, x2
    eret

el2:

    // set destination EL
    //mrs x0, spsr_el3
    //mov x1, #0x5 // EL1h
    //orr x0, x0, x1 // EL1h
    //msr spsr_el3, x0

    // set destination address
    //ldr x0, =el1
    //msr elr_el3, x0

    //eret
el1:
    // stop all processors besides cpu0
    mrs x1, mpidr_el1
    and x1, x1, #0xFF
    cmp x1, #0
    beq boot_cpu0

loop:
    wfi
    b loop

boot_cpu0:
    ldr x0, =STACKTOP
    mov sp, x0
    bl main

halt:
    wfi
    b halt
