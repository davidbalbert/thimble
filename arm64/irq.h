// There are three interrupt sources: GPU peripherals, ARM specific peripherals
// on the GPU, and ARM-local peripheral. In order to create one unified IRQ
// namespace, we create three separate domains so we can map hardware IRQs into
// trap numbers.

#define IRQ_DOMAIN 0xFF00
#define IRQ_NUM    0x00FF

#define IRQ_DOMAIN_LOCAL (1 << 8)
#define IRQ_DOMAIN_ARM   (2 << 8)
#define IRQ_DOMAIN_GPU   (3 << 8)
#define IRQ_DOMAIN_AUX   (4 << 8)

#define IRQ_TIMER (IRQ_DOMAIN_LOCAL | 1)
#define IRQ_AUX   (IRQ_DOMAIN_GPU   | 29)
#define IRQ_UART  (IRQ_DOMAIN_AUX   | 0)

#define IRQ_DMA0  (IRQ_DOMAIN_GPU   | 16)
#define IRQ_DMA(chan) (IRQ_DMA0 + chan)
