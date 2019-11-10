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

#define AUX_IRQ           ((volatile u32 *)(PBASE+0x215000))

#define LOCAL_IRQ_PENDING ((volatile u32 *)(LOCAL_PBASE+0x60))
#define TIMER_IRQ_CTL0 ((volatile u32 *)(LOCAL_PBASE+0x40))

#define IRQ_LOCAL_GPU 8 // IRQ 8 in the local domain means a GPU interrupt is pending

// 8 or 9 in the BASIC_PENDING register means you have to check the other registers
#define IRQ_BASIC_PENDING1 8
#define IRQ_BASIC_PENDING2 9

u32
readirq(void)
{
    u32 pending = *LOCAL_IRQ_PENDING;

    // In intenable, we go from IRQ number to the relevant bit in the pending
    // registers by left shifting by the IRQ number. To go in reverse, from a
    // bit to an interrupt number, we calculate the number of trailing zeroes
    // after the first set bit.
    //
    // If multiple bits are set, readirq returns trapno corresponding to the the
    // top bit.
    u32 hwirq = ctz(pending);

    if (hwirq != IRQ_LOCAL_GPU) {
        return IRQ_DOMAIN_LOCAL | hwirq;
    }

    pending = *IRQ_BASIC_PENDING;
    hwirq = ctz(pending);

    if (hwirq < IRQ_BASIC_PENDING1) {
        return IRQ_DOMAIN_ARM | hwirq;
    }

    if (hwirq > IRQ_BASIC_PENDING2) {
        switch (hwirq) {
            case 10:
                return IRQ_DOMAIN_GPU | 7;
            case 11:
                return IRQ_DOMAIN_GPU | 9;
            case 12:
                return IRQ_DOMAIN_GPU | 10;
            case 13:
                return IRQ_DOMAIN_GPU | 18;
            case 14:
                return IRQ_DOMAIN_GPU | 19;
            case 15:
                return IRQ_DOMAIN_GPU | 53;
            case 16:
                return IRQ_DOMAIN_GPU | 54;
            case 17:
                return IRQ_DOMAIN_GPU | 55;
            case 18:
                return IRQ_DOMAIN_GPU | 56;
            case 19:
                return IRQ_DOMAIN_GPU | 57;
            case 20:
                return IRQ_DOMAIN_GPU | 62;
            default:
                panic("readirq - unexpected irq in basic pending register %d", hwirq);
        }
    }

    if (hwirq == IRQ_BASIC_PENDING1) {
        pending = *IRQ_PENDING1;
        hwirq = ctz(pending);

        u32 irq = IRQ_DOMAIN_GPU | hwirq;

        if (irq != IRQ_AUX) {
            return irq;
        } else {
            pending = *AUX_IRQ;
            hwirq = ctz(pending);

            return IRQ_DOMAIN_AUX | hwirq;
        }
    }

    if (hwirq == IRQ_BASIC_PENDING2) {
        pending = *IRQ_PENDING2;
        hwirq = ctz(pending) + 32; // IRQ_PENDING2 maps to hwirq 32-63

        return IRQ_DOMAIN_GPU | hwirq;
    }

    panic("readirq - reached end without finding irq");
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
            if (hwirq <= 31) {
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
