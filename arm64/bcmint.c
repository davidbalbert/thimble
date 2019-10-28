// Broadcom 2837 interrupt controller

#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "irq.h"
#include "mem.h"

#include "bcm2837.h"

#define IRQ_BASE (PBASE+0xB000)

#define IRQ_BASIC_PENDING ((volatile u32 *)(IRQ_BASE+0x200))
#define IRQ_PENDING1      ((volatile u32 *)(IRQ_BASE+0x204))
#define IRQ_PENDING2      ((volatile u32 *)(IRQ_BASE+0x208))
#define IRQ_FIQ_CTRL      ((volatile u32 *)(IRQ_BASE+0x20C))
#define IRQ_ENABLE1       ((volatile u32 *)(IRQ_BASE+0x210))
#define IRQ_ENABLE2       ((volatile u32 *)(IRQ_BASE+0x214))
#define IRQ_BASIC_ENABLE  ((volatile u32 *)(IRQ_BASE+0x218))
#define IRQ_DISABLE1      ((volatile u32 *)(IRQ_BASE+0x21C))
#define IRQ_DISABLE2      ((volatile u32 *)(IRQ_BASE+0x220))
#define IRQ_BASIC_DISABLE ((volatile u32 *)(IRQ_BASE+0x224))

#define LOCAL_IRQ_PENDING ((volatile u32 *)(LOCAL_PBASE+0x60))

uchar
readirq(void)
{
    // TODO: GPU interrupts!
    //u32 pending = *IRQ_BASIC_PENDING;

    //cprintf("LOCAL_IRQ_PENDING: 0x%x\n", *LOCAL_IRQ_PENDING);
    //cprintf("IRQ_BASIC_PENDING: 0x%x\n", *IRQ_BASIC_PENDING);
    //cprintf("IRQ_PENDING1: 0x%x\n", *IRQ_PENDING1);
    //cprintf("IRQ_PENDING2: 0x%x\n", *IRQ_PENDING2);

    // In intenable, we go from IRQ number to the relevant bit in the pending
    // registers by left shifting by the IRQ number. To go in reverse, from a
    // bit to an interrupt number, we calculate the number of trailing zeroes
    // after the first set bit.
    //
    // If multiple bits are set, readirq returns the top bit.
    //uchar trapno = 64 - clz(pending) - 1;

    //cprintf("pending: %d\n", pending);
    //cprintf("trapno: %d\n", trapno);
    //cprintf("IRQ_ARM | trapno: %d\n", IRQ_ARM | trapno);

    //return IRQ_ARM | trapno;

    return *LOCAL_IRQ_PENDING;
}

void
intenable(uchar irq)
{
    int arm = irq & IRQ_ARM;
    uchar irq_noflag = irq & 0x3F;

    if (arm) {
        *IRQ_BASIC_ENABLE = (1 << irq_noflag);
    } else if (irq <= 31) {
        *IRQ_ENABLE1 = (1 << irq_noflag);
    } else {
        irq_noflag &= 0x1F; // mask out the top bit
        *IRQ_ENABLE2 = (1 << irq_noflag);
    }
}

void
intinit(void)
{
    // nothing to do here
}
