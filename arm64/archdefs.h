typedef struct TrapFrame TrapFrame;
typedef enum GpioAlt GpioAlt;
typedef enum GpioMode GpioMode;

// bcmint.c
u32 readirq(void);

// syscall.c
void syscall(TrapFrame *tf);

// uart.c
void handleuart(void);

// vmdbg.c
void kprintmap(Pte *pgdir);
void uprintmap(Pte *pgdir);
