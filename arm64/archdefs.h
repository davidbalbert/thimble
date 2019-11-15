typedef struct TrapFrame TrapFrame;

// bcmint.c
u32 readirq(void);

// sd.c
void sdinit(void);

// syscall.c
void syscall(TrapFrame *tf);

// uart.c
void handleuart(void);

// vmdbg.c
void kprintmap(Pte *pgdir);
void uprintmap(Pte *pgdir);
