struct Cpu {
    int id;                         // cpu id number
    int ncli;                       // number of times pushcli has been called.
    int intena;                     // whether interrupts were enabled before pushcli
    Registers *scheduler;
};
typedef struct Cpu Cpu;

struct Registers {
    u64 x19;
    u64 x20;
    u64 x21;
    u64 x22;
    u64 x23;
    u64 x24;
    u64 x25;
    u64 x26;
    u64 x27;
    u64 x28;
    u64 x29; // FP
    u64 x30; // LR
};
typedef struct Registers Registers;

typedef enum {
    UNUSED,
    EMBRYO,
    RUNNING,
    WAITING,
    READY,
} ProcState;

#define ERRMAX 1024
#define NFD    1024

typedef struct TrapFrame TrapFrame;

struct Proc {
    ProcState state;
    Registers *regs;
    Pte *pgmap;
    u8 *kstack; // base of the kernel stack
    usize sz; // total size in memory
    Proc *parent;
    int pid;
    TrapFrame *tf;
    char errstr[ERRMAX];
    File *files[NFD];
    int nextfd;
};
typedef struct Proc Proc;

extern Cpu  *cpu;   // current cpu
extern Proc *proc;  // current proc
