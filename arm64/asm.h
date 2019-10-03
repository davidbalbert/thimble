#define RES1 1

// https://developer.arm.com/docs/ddi0595/b/aarch64-system-registers/scr_el3

#define SCR_EL3_NS  1           // Non-secure bit - EL0 and EL1 are not in Secure state
#define SCR_EL3_SMD (1 << 7)    // Secure Monitor Call disable - disables SMC instruction
#define SCR_EL3_HCE (1 << 8)    // Hypervisor Call instruction enable - enables HVC instruction
#define SCR_EL3_RW  (1 << 10)   // EL2 is 64 bit, EL0 and EL1 can be 64 bit

#define SPSR_EL3_M_EL2H 0b1001   // return to EL2h mode - EL2 with EL2 stack (handler stack)
#define SPSR_EL3_F  (1 << 6)     // FIQ mask bit
#define SPSR_EL3_I  (1 << 7)     // IRQ mask bit
#define SPSR_EL3_A  (1 << 8)     // SError mask bit
#define SPSR_EL3_D  (1 << 9)     // Debug exception mask bit

#define HCR_EL2_RW  (1 << 31)    // EL1 is 64 bit, EL0 can be 64 bit

#define SPSR_EL2_M_EL1H 0b0101   // return to EL1h mode
#define SPSR_EL2_F  (1 << 6)     // FIQ mask
#define SPSR_EL2_I  (1 << 7)     // IRQ mask
#define SPSR_EL2_A  (1 << 8)     // SError mask
#define SPSR_EL2_D  (1 << 9)     // Debug mask

#define CPACR_EL1_FPEN_NOTRAP (0b11 << 20) // don't trap accesses to FP, SIMD in EL0 and EL1

#define CNTHCTL_EL2_EL1PCTEN 1        // don't trap access to physical counter register in EL0/1
#define CNTHCTL_EL2_EL1PCEN  (1 << 1) // don't trap access to physical timer register in EL0/1
