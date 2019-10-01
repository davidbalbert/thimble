#define RES1 1

// https://developer.arm.com/docs/ddi0595/b/aarch64-system-registers/scr_el3

#define SCR_EL3_NS  1           // Non-secure bit - EL0 and EL1 are not in Secure state
#define SCR_EL3_SMD (1 << 7)    // Secure Monitor Call disable - disables SMC instruction
#define SCR_EL3_HCE (1 << 8)    // Hypervisor Call instruction enable - enables HVC instruction
#define SCR_EL3_RW  (1 << 10)   // EL2 is 64 bit, EL0 and EL1 can be 64 bit

#define SPSR_EL3_M_EL2H 0b1001   // retrun to EL2h mode - EL2 with EL2 stack (handler stack)
#define SPSR_EL3_F  (1 << 6)     // FIQ mask bit
#define SPSR_EL3_I  (1 << 7)     // IRQ mask bit
#define SPSR_EL3_A  (1 << 8)     // SError mask bit
#define SPSR_EL3_D  (1 << 9)     // Debug exception mask bit

#define HCR_EL2_RW  (1 << 31)    // EL1 is 64 bit, EL0 can be 64 bit
