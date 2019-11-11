struct Cpu {
    int id;                         // cpu id number
    int ncli;                       // number of times pushcli has been called.
    int intena;                     // whether interrupts were enabled before pushcli
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
};
typedef struct Registers Registers;

typedef enum {
    UNUSED,
    EMBRYO,
    RUNNING,
    WAITING,
    READY,
} ProcState;

struct Proc {
    ProcState state;
    Registers *regs;
    Pte *pgmap;
    u8 *kstack; // base of the kernel stack
    usize sz; // total size in memory
    Proc *parent;
};
typedef struct Proc Proc;

extern Cpu  *cpu;   // current cpu
