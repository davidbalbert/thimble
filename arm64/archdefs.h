typedef struct Buf Buf;
typedef struct DmaControlBlock DmaControlBlock;
typedef struct TrapFrame TrapFrame;

// bcmint.c
u32 readirq(void);

// busaddr.c
u32 busaddr_mem(uintptr pa);
u32 busaddr_p(uintptr pa);

// dma.c
void dmainit(void);
void dmastart(int chan, int dev, int dir, void *src, void *dst, usize len);
//void handle_dma(void);

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
