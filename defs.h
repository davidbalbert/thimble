typedef struct SpinLock SpinLock;
typedef struct Registers Registers;
typedef struct Proc Proc;

// console.c
void cprintf(char *fmt, ...);
void cclear(void);
void cputs(char *s);
void cputc(uchar c);

// kalloc.c
void initmem1(void *start, void *end);
void initmem2(void *start, void *end);
void *kalloc(void);
void kfree(void *a);
void *pgfloor(void *addr);
void *p2v(ulong paddr);
ulong v2p(void *vaddr);

// klibc.c
void *memset(void *p, int c, usize len);
void *memzero(void *p, usize len);
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
void panic(char *s) __attribute__((noreturn));
void schedinit(void);
void scheduler(void) __attribute__((noreturn));
void start(void (*f)(void));
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
void kvmalloc(void);
void seginit(void);
Pml4e *setupkvm(void);
void switchkvm();
void switchuvm(Proc *p);

#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
