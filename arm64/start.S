#include "asm.h"

.global _start
_start:
    mrs x0, CurrentEL
    lsr x0, x0, #2 // shift EL down
    and x0, x0, #3 // clear other bits

    cmp x0, #3
    bne el2

    // Maybe we can get rid of SCR_EL3_HCE
    mov x1, #(SCR_EL3_RW | SCR_EL3_HCE | SCR_EL3_SMD | RES1 << 5 | RES1 << 4 | SCR_EL3_NS)
    msr SCR_EL3, x1

    mov x1, #(SPSR_EL3_D | SPSR_EL3_A | SPSR_EL3_I | SPSR_EL3_F | SPSR_EL3_M_EL2H)
    msr SPSR_EL3, x1

    adr x1, el2
    msr ELR_EL3, x1
    eret

el2:
    mrs x0, CurrentEL
    lsr x0, x0, #2
    and x0, x0, #3

    cmp x0, #2
    bne el1

    // enable CNTP for EL1 (TODO)
    mrs     x0, cnthctl_el2
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr

    // enable AArch64 in EL1
    mov x0, #(HCR_EL2_RW)
    orr x0, x0, #(RES1 << 1)
    msr HCR_EL2, x0

    // change execution level to EL1 (TODO)
    mov     x2, #0x3c4
    msr     spsr_el2, x2
    adr     x2, el1
    msr     elr_el2, x2

    eret

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
