#include "types.h"

#include "common.h"
#include "irq.h"
#include "mem.h"
#include "x86.h"

#define NIDT 256

struct TrapFrame {
    ulong rax;
    ulong rbx;
    ulong rcx;
    ulong rdx;
    ulong rbp;
    ulong rsi;
    ulong rdi;
    ulong r8;
    ulong r9;
    ulong r10;
    ulong r11;
    ulong r12;
    ulong r13;
    ulong r14;
    ulong r15;

    ulong trapno;
    ulong error;

    /* Pushed by hardware */
    ulong rip;
    ulong cs;
    ulong eflags;

    /* Pushed by hardware on privilege level change */
    ulong rsp;
    ulong ss;
};
typedef struct TrapFrame TrapFrame;

struct InterruptGate {
    uint offlow:16;     // bottom 16 bits of segment offset
    uint cs:16;         // code segment selector
    uint ist:3;         // interrupt stack table
    uint reserved1:5;   // all 0s.
    uint type:4;
    uint reserved2:1;   // 0
    uint dpl:2;         // descriptor privelege level
    uint p:1;           // present
    uint offmid:16;     // bits 16 - 31 of segment offset
    uint offhigh:32;    // bits 32 - 63 of segment offset
    uint reserved3:32;  // all 0s.
};
typedef struct InterruptGate InterruptGate;


static InterruptGate idt[NIDT] __attribute__((aligned(8)));

extern ulong vectors[];

ulong ticks;

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
trap(TrapFrame *tf)
{
    switch(tf->trapno) {
        case T_IRQ0 + IRQ_TIMER:
            handletimer();
            break;
        case T_IRQ0 + IRQ_KBD:
            handlekbd();
            break;
        default:
            cprintf("trap: %u, error: %u\n", tf->trapno, tf->error);
            break;
    }
}

void
trapinit(void)
{
    int i;

    for (i = 0; i < NIDT; i++) {
        // 0x8 refers to the code segment in the boot GDT. This may have to change later.
        mkgate(&idt[i], vectors[i], 0x8);
    }

    lidt(idt, sizeof(idt));
}
