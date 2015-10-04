#include "types.h"

#include "common.h"
#include "mem.h"
#include "x86.h"

#define NIDT 256

typedef struct {
    ulong trapno;
    ulong error;
} TrapFrame;

IntGate idt[NIDT] __attribute__((aligned(8)));
IdtDesc idtr;

extern ulong vectors[];

void
initidt(void)
{
    int i;

    for (i = 0; i < NIDT; i++) {
        // 0x8 refers to the code segment in the boot GDT. This may have to change later.
        INTGATE(idt[i], vectors[i], 0x8);
    }

    idtr = (IdtDesc){sizeof(idt) - 1, (ulong)&idt};
    lidt(&idtr);
}

void
trap(TrapFrame *tf)
{
    cprintf("trap: %u\n", tf->trapno);
}
