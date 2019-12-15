#include "u.h"

#include "archdefs.h"
#include "defs.h"
#include "irq.h"
#include "mem.h"
#include "x86.h"

#define NIDT 256

struct TrapFrame {
    u64 rax;
    u64 rbx;
    u64 rcx;
    u64 rdx;
    u64 rbp;
    u64 rsi;
    u64 rdi;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;

    u64 trapno;
    u64 error;

    /* Pushed by hardware */
    u64 rip;
    u16 cs;
    byte padding1[6];
    u64 rflags;
    u64 rsp;
    u16 ss;
    byte padding2[6];
};
typedef struct TrapFrame TrapFrame;

struct InterruptGate {
    u32 offlow:16;     // bottom 16 bits of segment offset
    u32 cs:16;         // code segment selector
    u32 ist:3;         // interrupt stack table
    u32 reserved1:5;   // all 0s.
    u32 type:4;
    u32 reserved2:1;   // 0
    u32 dpl:2;         // descriptor privelege level
    u32 p:1;           // present
    u32 offmid:16;     // bits 16 - 31 of segment offset
    u32 offhigh:32;    // bits 32 - 63 of segment offset
    u32 reserved3:32;  // all 0s.
};
typedef struct InterruptGate InterruptGate;


static InterruptGate idt[NIDT] __attribute__((aligned(8)));

extern u64 vectors[];

u64 ticks;

static void
mkgate(InterruptGate *gate, u64 offset, u16 selector)
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
            timerintr();
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
