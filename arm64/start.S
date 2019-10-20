#include "asm.h"
#include "mem.h"

.section ".text.boot"

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
    mov x0, #(TCR_SH0_INNER | TCR_ORGN0_WB_WA | TCR_IRGN0_WB_WA | TCR_TG0_4KB | (16 << TCR_T0SZ_SHIFT))
    orr x0, x0, #(16 << TCR_T1SZ_SHIFT)
    orr x0, x0, #(TCR_IRGN1_WB_WA)
    orr x0, x0, #(TCR_ORGN1_WB_WA)
    orr x0, x0, #(TCR_SH1_INNER)
    orr x0, x0, #(TCR_TG1_4KB)

    mov x1, #(TCR_IPS_48BIT)
    orr x0, x0, x1

    msr tcr_el1, x0

    // ATTR0 Device-nGnRnE
    // ATTR1 Device.
    // ATTR2 Normal Non-Cacheable.
    // ATTR3 Normal Cacheable.
    ldr x1, =0xFF440400
    msr mair_el1, x1

    adr x0, boot_pgtable_0
    msr ttbr0_el1, x0
    msr ttbr1_el1, x0

    isb

    mrs x0, sctlr_el1
    orr x0, x0, #(SCTLR_EL1_I)
    orr x0, x0, #(SCTLR_EL1_C)
    orr x0, x0, #(SCTLR_EL1_M)
    msr sctlr_el1, x0
    dsb sy
    isb

    ldr x0, =bootkstack
    add x0, x0, KSTACKSIZE
    mov sp, x0

    ldr x0, =main
    blr x0

halt:
    wfi
    b halt

.macro pg_table_entry pa, attr
    .quad \pa + (PTE_TABLE | PTE_P)
.endm

.macro pg_table_block pa, attr
    .quad \pa | \attr | PTE_BLOCK | PTE_P
.endm

// Boot page tables. Enough to jump to main in the upper half.

.align 12 // 4KB
boot_pgtable_0:
pg_table_entry boot_pgtable_1, 0
.rept 511
.quad 0
.endr

.align 12
boot_pgtable_1:
pg_table_entry boot_pgtable_2, 0
.rept 511
.quad 0
.endr

// Map 16 MB of memory, as well as enough device space to use uart1
.align 12
boot_pgtable_2:
pg_table_block 0x00000000, PTE_AF | PTE_ISH | PTE_CACHEABLE
pg_table_block 0x00200000, PTE_AF | PTE_ISH | PTE_CACHEABLE
pg_table_block 0x00400000, PTE_AF | PTE_ISH | PTE_CACHEABLE
pg_table_block 0x00600000, PTE_AF | PTE_ISH | PTE_CACHEABLE
pg_table_block 0x00800000, PTE_AF | PTE_ISH | PTE_CACHEABLE
pg_table_block 0x00A00000, PTE_AF | PTE_ISH | PTE_CACHEABLE
pg_table_block 0x00C00000, PTE_AF | PTE_ISH | PTE_CACHEABLE
pg_table_block 0x00E00000, PTE_AF | PTE_ISH | PTE_CACHEABLE
pg_table_block 0x01000000, PTE_AF | PTE_ISH | PTE_CACHEABLE
.rept 495
.quad 0
.endr
pg_table_block 0x3F000000, PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE
pg_table_block 0x3F200000, PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE
pg_table_block 0x3F400000, PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE
pg_table_block 0x3F600000, PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE
pg_table_block 0x3F800000, PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE
pg_table_block 0x3FA00000, PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE
pg_table_block 0x3FC00000, PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE
pg_table_block 0x3FE00000, PTE_AF | PTE_ISH | PTE_DEVICE_nGnRnE
