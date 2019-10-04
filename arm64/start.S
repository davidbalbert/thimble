#include "asm.h"

.global _start
_start:
    mrs x0, CurrentEL
    lsr x0, x0, #2 // shift EL down
    and x0, x0, #3 // clear other bits

    cmp x0, #3
    bne el2

    // Maybe we can get rid of SCR_EL3_HCE
    mov x1, #(SCR_EL3_RW | SCR_EL3_SMD | RES1 << 5 | RES1 << 4 | SCR_EL3_NS)
    msr scr_el3, x1

    mov x1, #(SPSR_EL3_D | SPSR_EL3_A | SPSR_EL3_I | SPSR_EL3_F | SPSR_EL3_M_EL2H)
    msr spsr_el3, x1

    adr x1, el2
    msr elr_el3, x1
    eret

el2:
    mrs x0, CurrentEL
    lsr x0, x0, #2 // shift EL down
    and x0, x0, #3 // clear other bits

    cmp x0, #2
    bne el1

    // initialize sctlr_el1
    msr sctlr_el1, xzr

    // enable CNTP for EL1
    mov x0, #(CNTHCTL_EL2_EL1PCEN | CNTHCTL_EL2_EL1PCTEN)
    msr cnthctl_el2, x0
    msr cntvoff_el2, xzr

    // Don't trap floating point registers
    mov x0, #(CPACR_EL1_FPEN_NOTRAP)
    msr cpacr_el1, x0

    // enable AArch64 in EL1
    mov x0, #(HCR_EL2_RW | HCR_EL2_HCD)
    orr x0, x0, #(RES1 << 1)
    msr hcr_el2, x0

    // change execution level to EL1
    mov x2, #(SPSR_EL2_D | SPSR_EL2_A | SPSR_EL2_I | SPSR_EL2_F | SPSR_EL2_M_EL1H)
    msr spsr_el2, x2
    adr x2, el1
    msr elr_el2, x2

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
    // basic memory management
    mov x0, #(16 << TCR_T0SZ_SHIFT)
    orr x0, x0, #(16 << TCR_T1SZ_SHIFT)
    // TODO: continue mmu setup

    ldr x0, =STACKTOP
    mov sp, x0
    bl main

halt:
    wfi
    b halt
