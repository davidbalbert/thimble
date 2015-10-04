#include "types.h"

#include "common.h"
#include "mem.h"
#include "x86.h"

#define NIDT 256

typedef struct {
    ulong trapno;
    ulong error;
} TrapFrame;

static InterruptGate idt[NIDT] __attribute__((aligned(8)));
static IdtDesc idtr;

extern ulong vectors[];

static void
mkgate(InterruptGate *gate, ulong offset, ushort selector)
{
    gate->offlow = offset & 0xFFFF;
    gate->cs = selector;
    gate->ist = 0;
    gate->reserved1 = 0;
    gate->type = 0b1110;
    gate->reserved2 = 0;
    gate->dpl = 0;
    gate->p = 1;
    gate->offmid = (offset >> 16) & 0xFFFF;
    gate->offhigh = (offset >> 32);
    gate->reserved3 = 0;
}

void
trapinit(void)
{
    int i;

    for (i = 0; i < NIDT; i++) {
        // 0x8 refers to the code segment in the boot GDT. This may have to change later.
        mkgate(&idt[i], vectors[i], 0x8);
    }

    idtr = (IdtDesc){sizeof(idt) - 1, (ulong)&idt};
    lidt(&idtr);
}

void
trap(TrapFrame *tf)
{
    cprintf("trap: %u, error: %u\n", tf->trapno, tf->error);
}
