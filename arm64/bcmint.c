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
#define TIMER_IRQ_CTL0 ((volatile u32 *)(LOCAL_PBASE+0x40))

u32
readirq(void)
{
    u32 pending = *LOCAL_IRQ_PENDING;

    // In intenable, we go from IRQ number to the relevant bit in the pending
    // registers by left shifting by the IRQ number. To go in reverse, from a
    // bit to an interrupt number, we calculate the number of trailing zeroes
    // after the first set bit. A64 doesn't have a count trailing zeroes
    // instruction, but we can build one from the CLZ (count leading zeroes)
    // instruction
    //
    // If multiple bits are set, readirq returns trapno corresponding to the the
    // top bit.
    //
    // clz takes a u64, so we subtract clz from 64 instead of 32.
    u32 hwirq = 64 - clz(pending) - 1;

    return IRQ_DOMAIN_LOCAL | hwirq;
}

static void
enablelocal(u32 hwirq)
{
    if (hwirq >= 0 && hwirq <= 3) {
        // TODO: multicore
        *TIMER_IRQ_CTL0 = (1 << hwirq);
    } else {
        panic("enablelocal - unknown local IRQ %d", hwirq);
    }
}

void
intenable(u32 irq)
{
    int domain = irq & IRQ_DOMAIN;
    int hwirq = irq & IRQ_NUM;

    switch (domain) {
        case IRQ_DOMAIN_LOCAL:
            enablelocal(hwirq);
            break;
        case IRQ_DOMAIN_ARM:
            *IRQ_BASIC_ENABLE = (1 << hwirq);
            break;
        case IRQ_DOMAIN_GPU:
            if (irq <= 31) {
                *IRQ_ENABLE1 = (1 << hwirq);
            } else {
                hwirq &= 0x1F; // mask out the top bit
                *IRQ_ENABLE2 = (1 << hwirq);
            }
            break;
        default:
            panic("unknown IRQ domain %d", domain);
            break;
    }
}

void
intinit(void)
{
    // nothing to do here
}
