#define RES1 1

#define SCR_EL3_RW  (1 << 10)   // EL2 is 64 bit, EL0 and EL1 can be 64 bit
#define SCR_EL3_SMD (1 << 7)    // Secure Monitor Call disable - disables SMC instruction
#define SCR_EL3_NS  1           // Non-secure bit - EL0 and EL1 are not in Secure state

#define SPSR_EL3_D  (1 << 9)     // Debug exception mask bit
#define SPSR_EL3_A  (1 << 8)     // SError mask bit
#define SPSR_EL3_I  (1 << 7)     // IRQ mask bit
#define SPSR_EL3_F  (1 << 6)     // FIQ mask bit
#define SPSR_EL3_M_EL2H 0b1001   // return to EL2h mode - EL2 with EL2 stack (handler stack)

#define HCR_EL2_RW  (1 << 31)    // EL1 is 64 bit, EL0 can be 64 bit
#define HCR_EL2_HCD (1 << 29)    // Disables HVC instruction

#define SPSR_EL2_D  (1 << 9)     // Debug mask
#define SPSR_EL2_A  (1 << 8)     // SError mask
#define SPSR_EL2_I  (1 << 7)     // IRQ mask
#define SPSR_EL2_F  (1 << 6)     // FIQ mask
#define SPSR_EL2_M_EL1H 0b0101   // return to EL1h mode

#define SPSR_EL1_D  (1 << 9)     // Debug mask
#define SPSR_EL1_A  (1 << 8)     // SError mask
#define SPSR_EL1_I  (1 << 7)     // IRQ mask
#define SPSR_EL1_F  (1 << 6)     // FIQ mask
#define SPSR_EL1_M_EL1H 0b0101   // return to EL1h mode


#define CPACR_EL1_FPEN_NOTRAP (0b11 << 20) // don't trap accesses to FP, SIMD in EL0 and EL1

#define CNTHCTL_EL2_EL1PCTEN 1        // don't trap access to physical counter register in EL0/1
#define CNTHCTL_EL2_EL1PCEN  (1 << 1) // don't trap access to physical timer register in EL0/1

#define TCR_T0SZ_SHIFT 0
#define TCR_T1SZ_SHIFT 16

#define TCR_IRGN0_WB_WA (0b01 << 8)  // Inner cacheability - Write-Back Read-Allocate Write-Allocate cacheable
#define TCR_ORGN0_WB_WA (0b01 << 10) // Outer cacheability - Write-Back Read-Allocate Write-Allocate cacheable
#define TCR_SH0_INNER (0b11 << 12)   // inner sharable
#define TCR_TG0_4KB (0 << 14)        // 4KB granules in TTBR0_EL1. don't actually need this, but it makes things clearer
#define TCR_IRGN1_WB_WA (0b01 << 24)
#define TCR_ORGN1_WB_WA (0b01 << 26)
#define TCR_SH1_INNER (0b11 << 28)
#define TCR_TG1_4KB (0b10 << 30)     // 4KB granules in TTBR1_EL1
#define TCR_IPS_48BIT (0b101 << 32)  // 48 bit intermediate physical address size

#define SCTLR_EL1_M 1         // MMU enable
#define SCTLR_EL1_C (1 << 2)  // data cache enable
#define SCTLR_EL1_I (1 << 12) // instruction cache enable

#define DAIF_D (1 << 9)
#define DAIF_A (1 << 8)
#define DAIF_I (1 << 7)
#define DAIF_F (1 << 6)

#define DAIF_ALL (DAIF_D | DAIF_A | DAIF_I | DAIF_F)
