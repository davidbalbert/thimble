typedef struct Buf Buf;
typedef struct DmaControlBlock DmaControlBlock;
typedef struct TrapFrame TrapFrame;

// bcmint.c
u32 readirq(void);

// dma.c
void dma_init(void);
void dma_start(int chan, DmaControlBlock *cb);

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
