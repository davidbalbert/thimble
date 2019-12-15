typedef struct Buf Buf;
typedef struct DmaControlBlock DmaControlBlock;
typedef struct TrapFrame TrapFrame;
typedef struct SpinLock SpinLock;

// bcmint.c
u32 readirq(void);

// busaddr.c
u32 busaddr_mem(uintptr pa);
u32 busaddr_p(uintptr pa);

// dma.c
void dmastart(int chan, int dev, int dir, void *src, void *dst, usize len);
void dmawait(int chan, SpinLock *lock);
void dmaintr(int chan);

// sd.c
void sdinit(void);
void sdrw(Buf *b, int write);

// syscall.c
void syscall(TrapFrame *tf);

// uart.c
void uartintr(void);

// vmdbg.c
void kprintmap(Pte *pgdir);
void uprintmap(Pte *pgdir);
