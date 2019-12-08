typedef struct Cpu Cpu;

struct SpinLock {
    u32 locked;

    char *name;
    Cpu *cpu;
};

typedef struct SpinLock SpinLock;
