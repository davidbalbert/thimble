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
    uint db:1;          // default operation size (16 bit/32 bit segment)
    uint g:1;           // Granularity
    uint base24_31:8;   // top byte of segment base;
};
typedef struct SegmentDescriptor SegmentDescriptor;

struct TaskStateDescriptor {
    // First 8 bytes are the same as SegmentDescriptor
    uint limit0_15:16;  // bottom 16 bits of segment limit
    uint base0_15:16;   // bottom 16 bits of base
    uint base16_23:8;   // third byte of base
    uint type:4;
    uint s:1;           // descriptor type 0 = system, 1 = code or data
    uint dpl:2;         // descriptor privlege level
    uint p:1;           // segment present
    uint limit16_19:4;  // bits 16 - 19 of the segment limit
    uint avl:1;         // available for use by system software
    uint l:1;           // 64 bit code segment
    uint db:1;          // default operation size (16 bit/32 bit segment)
    uint g:1;           // Granularity
    uint base24_31:8;   // top byte of segment base;

    uint base32_63;     // top four bytes of base
    uint reserved;      // bits 8-12 (type and S) must be zero so CPU doesn't get confused
};
typedef struct TaskStateDescriptor TaskStateDescriptor;

struct __attribute__((packed)) TaskState  {
    uint reserved1;
    ulong rsp0;         // stack pointers for each privilege level
    ulong rsp1;
    ulong rsp2;
    ulong reserved2;
    ulong ist1;         // interrupt stack table pointers
    ulong ist2;
    ulong ist3;
    ulong ist4;
    ulong ist5;
    ulong ist6;
    ulong ist7;
    ulong reserved3;
    ushort rserved4;
    ushort iomapbase;   // offset from the TSS base to the I/O permission bit map from the
};
typedef struct TaskState TaskState;

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

// 1 null descriptor
// 2 kernel segments
// 3 user segments (includes one unused compatibility mode code segment descriptor)
// 1 128-bit TSS descriptor
#define NDESC 8

struct Cpu {
    Registers *scheduler;           // swtch to this to call the scheduler
    SegmentDescriptor gdt[NDESC];
    TaskState ts;                   // Used for interrupt stack switching
    int id;                         // cpu id number
    int ncli;                       // number of times pushcli has been called.
    int intena;                     // whether interrupts were enabled before pushcli
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
    uchar *ustack;
    Pml4e *pgmap;
};
typedef struct Proc Proc;

extern Cpu  *cpu;   // current cpu
extern Proc *proc;  // current process
