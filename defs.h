typedef unsigned long  Pml4e;
typedef unsigned long  Pdpe;
typedef unsigned long  Pde;
typedef unsigned long  Pte;

typedef struct SpinLock SpinLock;
typedef struct Registers Registers;
typedef struct Proc Proc;

// console.c
void cprintf(char *fmt, ...);
void cclear(void);
void cputs(char *s);
void cputc(uchar c);
void cvprintf(char *fmt, va_list ap);

// kalloc.c
void initmem1(void *start, void *end);
void initmem2(void *start, void *end);
void *kalloc(void);
void kfree(void *a);
void *pgfloor(void *addr);
void *pgceil(void *addr);
void *p2v(ulong paddr);
ulong v2p(void *vaddr);

// klibc.c
void *memmove(void *dst, void *src, usize n);
void *memset(void *p, int c, usize n);
void *memzero(void *p, usize n);
int isdigit(int c);
long strtol(char *s, char **endptr, int base);

// kbd.c
void kbdinit(void);
void handlekbd(void);

// lock.c
void initlock(SpinLock *l);
void lock(SpinLock *l);
void unlock(SpinLock *l);
void pushcli(void);
void popcli(void);

// pic.c
void picinit(void);
void picenable(uchar irq);

// proc.c
void panic(char *fmt, ...) __attribute__((noreturn));
void schedinit(void);
void scheduler(void) __attribute__((noreturn));
void start(uchar *data, usize size);
void yield(void);

// switch.S
void swtch(Registers **from, Registers *to);

// syscallasm.S
void sysinit(void);

// timer.c
void timerinit(void);
void handletimer(void);

// trap.c
void trapinit(void);

// vm.c
usize allocuvm(Pml4e *pgmap, usize oldsz, usize newsz);
void clearpteu(Pml4e *pgmap, void *addr);
void kvmalloc(void);
void loaduvm(Pml4e *pgmap, char *addr, uchar *data, usize sz);
void seginit(void);
Pml4e *setupkvm(void);
void switchkvm();
void switchuvm(Proc *p);
void *uva2ka(Pml4e *pgmap, void *addr);
Pte *walkpgmap(Pml4e *pgmap, void *va, int alloc);

#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
