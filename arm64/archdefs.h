typedef struct TrapFrame TrapFrame;

// arm64/syscall.c
void syscall(TrapFrame *tf);

// bcmint.c
u32 readirq(void);

// uart.c
void handleuart(void);

// vmdbg.c
void kprintmap(Pte *pgdir);
void uprintmap(Pte *pgdir);
