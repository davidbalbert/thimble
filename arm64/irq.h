// There are three interrupt sources, the GPU and the ARM. ARM interrupts are
// numbered 0-7 and are available in the IRQ_BASIC_* registers. GPU interrupts
// are numbered 0-63 and are available in the IRQ_*_1 and IRQ_*_2 registers
// (select GPU interrupts are also available in IRQ_BASIC_*).
//
// We create a unified numbering scheme where GPU interrupts are 0-63 and ARM
// interrupts are 64-71.
#define IRQ_ARM (1 << 6) // ARM peripheral interrupts
#define IRQ_GPU (0 << 6)

#define IRQ_TIMER (IRQ_ARM | 0)

#define TIMER_IRQ_CTL_nCNTPNSIRQ (1 << 1) // EL1 Non-secure physical timer
