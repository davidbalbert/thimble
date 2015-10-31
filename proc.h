struct SegmentDescriptor {
    uint limit0_15:16;  // bottom 16 bits of segment limit
    uint base0_15:16;   // bottom 16 bits of segment base
    uint base16_23:8;   // third byte of segment base
    uint type:4;
    uint s:1;           // descriptor type 0 = system, 1 = code or data
    uint dpl:2;         // descriptor privlege level
    uint p:1;           // segment present
    uint limit16_19:4;  // bits 16 - 19 of the segment limit
    uint avl:1;         // available for use by system software
    uint l:1;           // 64 bit code segment
    uint db:1;          // default operation size (16 bit/32 bit segment).
    uint g:1;           // Granularity
    uint base24_31:8;   // top byte of segment base;
};
typedef struct SegmentDescriptor SegmentDescriptor;

/*
 * Callee saved registers: rbp, rbx, and r12-r15
 *
 * Instruction pointer is stored on the stack by swtch's caller and thus is not
 * pushed onto the stack explicitly.
 */
struct Registers {
    ulong rbp;
    ulong rbx;
    ulong r12;
    ulong r13;
    ulong r14;
    ulong r15;
    ulong rip;
};
typedef struct Registers Registers;

struct Cpu {
    Registers *scheduler;       // swtch to this to call the scheduler
    SegmentDescriptor gdt[5];
    int id;                     // cpu id number
    int ncli;                   // number of times pushcli has been called.
    int intena;                 // whether interrupts were enabled before pushcli
};
typedef struct Cpu Cpu;

typedef enum {
    RUNNING,
    WAITING,
    READY,
} ProcState;

struct Proc {
    ProcState state;
    Registers *regs;
    uchar *kstack;
};
typedef struct Proc Proc;

extern Cpu  *cpu;   // current cpu
extern Proc *proc;  // current process
