#define RES1 1

// https://developer.arm.com/docs/ddi0595/b/aarch64-system-registers/scr_el3

#define SCR_EL3_RW  (1 << 10)   // EL2 is 64 bit, EL0 and EL1 can be 64 bit
#define SCR_EL3_HCE (1 << 8)    // Hypervisor Call instruction enable - enables HVC instruction
#define SCR_EL3_SMD (1 << 7)    // Secure Monitor Call disable - disables SMC instruction
#define SCR_EL3_NS  1           // Non-secure bit - EL0 and EL1 are not in Secure state
