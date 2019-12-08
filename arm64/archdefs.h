typedef struct TrapFrame TrapFrame;
typedef struct Buf Buf;

// bcmint.c
u32 readirq(void);

// sd.c
void sdinit(void);
void sdrw(Buf *b, int write);

// syscall.c
void syscall(TrapFrame *tf);

// uart.c
void handleuart(void);

// vmdbg.c
void kprintmap(Pte *pgdir);
void uprintmap(Pte *pgdir);
