#include "types.h"

#include "defs.h"
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
    ushort cs;
    uchar padding1[6];
    ulong rflags;
    ulong rsp;
    ushort ss;
    uchar padding2[6];
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
dumpregs(TrapFrame *tf) {
    cprintf("rip: 0x%016x, cs:  0x%016x, rflags: 0x%016x\n",
            tf->rip, tf->cs, tf->rflags);
    cprintf("rsp: 0x%016x, ss:  0x%016x, cr2: 0x%016x\n",
            tf->rsp, tf->ss, readcr2());
    cprintf("rax: 0x%016x, rbx: 0x%016x, rcx: 0x%016x\n",
            tf->rax, tf->rbx, tf->rcx);
    cprintf("rdx: 0x%016x, rdi: 0x%016x, rsi: 0x%016x\n",
            tf->rdx, tf->rdi, tf->rsi);
    cprintf("r8:  0x%016x, r9:  0x%016x, r10: 0x%016x\n",
            tf->r8, tf->r9, tf-> r10);
    cprintf("r11: 0x%016x, r12: 0x%016x, r13: 0x%016x\n",
            tf->r11, tf->r12, tf->r13);
    cprintf("r14: 0x%016x, r15: 0x%016x, rbp: 0x%016x\n",
            tf->r14, tf->r15, tf->rbp);
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
            dumpregs(tf);
            for (;;)
                hlt();
            break;
    }
}

void
trapinit(void)
{
    int i;

    for (i = 0; i < NIDT; i++) {
        mkgate(&idt[i], vectors[i], SEG_KCODE << 3);
    }

    lidt(idt, sizeof(idt));
}
