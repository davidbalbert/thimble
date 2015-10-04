#include "types.h"

#include "common.h"
#include "mem.h"
#include "x86.h"

#define NIDT 256

typedef struct {
    ulong trapno;
    ulong error;
} TrapFrame;

IDTEntry idt[NIDT] __attribute__((aligned(8)));
IDTDesc idtr;

extern ulong vectors[];

void
initidt(void)
{
    int i;

    for (i = 0; i < NIDT; i++) {
        idt[i].offlow = vectors[i] & 0xFFFF;
        idt[i].cs = 0x8; // Hard coded from boot GDT. Might need to change later.
        idt[i].ist = 0;
        idt[i].reserved1 = 0;
        idt[i].type = 0b1110;
        idt[i].reserved2 = 0;
        idt[i].dpl = 0;
        idt[i].p = 1;
        idt[i].offmid = (vectors[i] >> 16) & 0xFFFF;
        idt[i].offhigh = (vectors[i] >> 32);
        idt[i].reserved3 = 0;
    }

    idtr = (IDTDesc){sizeof(idt) - 1, (ulong)&idt};
    lidt(&idtr);
}

void
trap(TrapFrame *tf)
{
    cprintf("trap: %x\n", tf->trapno);
}
