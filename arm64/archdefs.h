typedef struct TrapFrame TrapFrame;

// bcmint.c
u32 readirq(void);

// sd.c
void sdinit(void);
void sdread(byte *addr, u64 lba, u16 count);

// syscall.c
void syscall(TrapFrame *tf);

// uart.c
void handleuart(void);

// vmdbg.c
void kprintmap(Pte *pgdir);
void uprintmap(Pte *pgdir);
