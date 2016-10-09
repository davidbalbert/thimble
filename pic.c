#include "u.h"

#include "irq.h"
#include "x86.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

#define PIC_EOI 0x20

// Initial IRQ mask
static ushort irqmask = 0xFFFF & ~(1<<IRQ_PIC2);

static void
setmask(ushort mask)
{
    irqmask = mask;
    outb(PIC1_DATA, mask);
    outb(PIC2_DATA, mask >> 8);
}

void
picenable(uchar irq)
{
    setmask(irqmask & ~(1 << irq));
}

void
picinit(void)
{
    // mask all interrupts
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    // ICW1
    //  0x10 = init
    //  0x01 = ICW4 required
    outb(PIC1_CMD, 0x11);

    // ICW2: offset
    outb(PIC1_DATA, T_IRQ0);

    // ICW3:
    //  primary - bit mask of lines connected to secondary
    //  secondary - Secondary's IRQ on primary
    outb(PIC1_DATA, 1 << IRQ_PIC2);

    // ICW4:
    //  0x1 = x86 mode
    //  0x2 = automatic EOI mode
    outb(PIC1_DATA, 0x3);


    // ICW2: offset
    outb(PIC2_CMD, 0x11);
    outb(PIC2_DATA, T_IRQ0 + 8);
    outb(PIC2_DATA, IRQ_PIC2);

    // xv6 source says Auto EOI doesn't work on secondary,
    // but still enables it. Hmm...
    outb(PIC2_DATA, 0x3);

    setmask(irqmask);
}
