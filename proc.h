typedef enum {
    RUNNING,
    WAITING,
    READY,
} ProcState;

struct Cpu {
    Registers *scheduler;       // swtch to this to call the scheduler
    int ncli;                   // number of times pushcli has been called.
    int intena;                 // whether interrupts were enabled before pushcli
};
typedef struct Cpu Cpu;

struct Proc {
    ProcState state;
    Registers *regs;
    uchar *kstack;
};
typedef struct Proc Proc;

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

extern Cpu  *cpu;   // current cpu
extern Proc *proc;  // current process
