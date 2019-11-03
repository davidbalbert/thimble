typedef struct SpinLock SpinLock;
typedef struct Registers Registers;
typedef struct Proc Proc;
typedef struct SyscallFrame SyscallFrame;

// console.c

typedef struct Console Console;

Console *uart_console;
Console *vga_console;

void cinit(Console *c);
void cprintf(char *fmt, ...);
void cclear(void);
void cputs(char *s);
void cputc(uchar c);
void cvprintf(char *fmt, va_list ap);
void cwrite(char *buf, usize nbytes);

// $(ARCH)/arch.c

void archinit_console(void);
void archmain(void);

// $(ARCH)/cpu.c

void halt(void);

// file.c
void fileinit(void);
void copyfds(Proc *oldp, Proc *newp);

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
int strcmp(char *s1, char *s2);
usize strlen(char *s);
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

// pic.c/bcmint.c
void intinit(void);
void intenable(uchar irq);
uchar readirq(void);

// proc.c
void panic(char *fmt, ...) __attribute__((noreturn));
void vpanic(char *fmt, va_list ap) __attribute__((noreturn));
void schedinit(void);
void scheduler(void) __attribute__((noreturn));
void mkproc(uchar *data);
void yield(void);

// switch.S
void swtch(Registers **from, Registers *to);

// syscall.c
long argfd(SyscallFrame *f, int n, int *fd);
long argint(SyscallFrame *f, int n, int *ip);
long arglong(SyscallFrame *f, int n, long *lp);
long argptr(SyscallFrame *f, int n, uintptr *p, usize size);
long argstr(SyscallFrame *f, int n, char **pp);

// syscallasm.S
void sysinit(void);

// timer.c
void timerinit(void);
void handletimer(void);

// trap.c
void trapinit(void);

// vm.c
usize allocuvm(Pte *pgmap, usize oldsz, usize newsz);
void clearpteu(Pte *pgmap, void *addr);
Pte* copyuvm(Pte *oldmap, usize sz);
void freeuvm(Pte *pgmap);
void kvmalloc(void);
void loaduvm(Pte *pgmap, char *addr, uchar *data, usize sz);
void seginit(void);
Pte *setupkvm(void);
void switchkvm();
void switchuvm(Proc *p);
void *uva2ka(Pte *pgmap, void *addr);

// vmdbg.c
void printmap(Pte *pgdir);
