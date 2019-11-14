struct SegmentDescriptor {
    u32 limit0_15:16;  // bottom 16 bits of segment limit
    u32 base0_15:16;   // bottom 16 bits of segment base
    u32 base16_23:8;   // third byte of segment base
    u32 type:4;
    u32 s:1;           // descriptor type 0 = system, 1 = code or data
    u32 dpl:2;         // descriptor privlege level
    u32 p:1;           // segment present
    u32 limit16_19:4;  // bits 16 - 19 of the segment limit
    u32 avl:1;         // available for use by system software
    u32 l:1;           // 64 bit code segment
    u32 db:1;          // default operation size (16 bit/32 bit segment)
    u32 g:1;           // Granularity
    u32 base24_31:8;   // top byte of segment base;
};
typedef struct SegmentDescriptor SegmentDescriptor;

struct TaskStateDescriptor {
    // First 8 bytes are the same as SegmentDescriptor
    u32 limit0_15:16;  // bottom 16 bits of segment limit
    u32 base0_15:16;   // bottom 16 bits of base
    u32 base16_23:8;   // third byte of base
    u32 type:4;
    u32 s:1;           // descriptor type 0 = system, 1 = code or data
    u32 dpl:2;         // descriptor privlege level
    u32 p:1;           // segment present
    u32 limit16_19:4;  // bits 16 - 19 of the segment limit
    u32 avl:1;         // available for use by system software
    u32 l:1;           // 64 bit code segment
    u32 db:1;          // default operation size (16 bit/32 bit segment)
    u32 g:1;           // Granularity
    u32 base24_31:8;   // top byte of segment base;

    u32 base32_63;     // top four bytes of base
    u32 reserved;      // bits 8-12 (type and S) must be zero so CPU doesn't get confused
};
typedef struct TaskStateDescriptor TaskStateDescriptor;

struct __attribute__((packed)) TaskState  {
    u32 reserved1;
    u64 rsp0;         // stack pointers for each privilege level
    u64 rsp1;
    u64 rsp2;
    u64 reserved2;
    u64 ist1;         // interrupt stack table pointers
    u64 ist2;
    u64 ist3;
    u64 ist4;
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u64 reserved3;
    u16 rserved4;
    u16 iomapbase;   // offset from the TSS base to the I/O permission bit map from the
};
typedef struct TaskState TaskState;

/*
 * Callee saved registers: rbp, rbx, and r12-r15
 *
 * Instruction pointer is stored on the stack by swtch's caller and thus is not
 * pushed onto the stack explicitly.
 */
struct Registers {
    u64 rbp;
    u64 rbx;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
    u64 rip;
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
    int noff;                       // number of times push_off has been called.
    int intena;                     // whether interrupts were enabled before push_off
};
typedef struct Cpu Cpu;

typedef enum {
    UNUSED,
    EMBRYO,
    RUNNING,
    WAITING,
    READY,
} ProcState;

#define ERRMAX 1024
#define NFD    1024

typedef struct File File;

struct SyscallFrame {
    long rax;           // syscall return value
    u64 rsp;          // user stack pointer
    u64 num;
    u64 args[6];
};
typedef struct SyscallFrame SyscallFrame;

typedef struct Proc Proc;
struct Proc {
    ProcState state;
    Registers *regs;
    SyscallFrame *sf;
    int pid;
    byte *kstack;      // base of the kernel stack
    Pte *pgmap;
    usize sz;           // total size in memory
    char errstr[ERRMAX];
    File *files[NFD];
    int nextfd;
    Proc *parent;
};

extern Cpu  *cpu;   // current cpu
extern Proc *proc;  // current process
