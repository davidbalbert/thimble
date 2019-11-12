typedef struct TrapFrame TrapFrame;

// arm64/syscall.c
void syscall(TrapFrame *tf);

// proc.c
void start(void (*f)(void));
