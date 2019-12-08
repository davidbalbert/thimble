typedef struct Buf Buf;
typedef struct Cpu Cpu;
typedef struct SpinLock SpinLock;
typedef struct Registers Registers;
typedef struct Proc Proc;
typedef struct File File;

// buf.o
void binit(void);
Buf *bread(uint dev, u64 blockno);
void brelse(Buf *b);
void bwrite(Buf *b);

// console.c

typedef struct Console Console;

Console *uart_console;
Console *vga_console;

void cinit(Console *c);
void cprintf(char *fmt, ...);
void cclear(void);
void cputs(char *s);
void cputc(byte c);
void cvprintf(char *fmt, va_list ap);
void cwrite(char *buf, usize nbytes);

// $(ARCH)/cpu.c
void intr_on(void);
void intr_off(void);

// $(ARCH)/arch.c

void archinit_early(void);
void archinit(void);

// $(ARCH)/cpu.c

void halt(void);

// file.c
void fileinit(void);
void copyfds(Proc *oldp, Proc *newp);

// kalloc.c
void initmem1(void *start, void *end);
void initmem2(void *end);
void *kalloc(void);
void kfree(void *a);
void *pgfloor(void *addr);
void *pgceil(void *addr);
void *p2v(u64 paddr);
u64 v2p(void *vaddr);

// klibc.c
void *memmove(void *dst, void *src, usize n);
void * memcpy(void *dst, void *src, usize n);
void *memset(void *p, int c, usize n);
void *memzero(void *p, usize n);
int isdigit(int c);
int isprint(int c);
int strcmp(char *s1, char *s2);
usize strlen(char *s);
long strtol(char *s, char **endptr, int base);

// lock.c
void initlock(SpinLock *l, char *name);
void lock(SpinLock *l);
void unlock(SpinLock *l);
void push_off(void);
void pop_off(void);
int holding(SpinLock *l);

// pic.c/bcmint.c
void intinit(void);
void intenable(u32 irq);

// proc.c
Cpu *mycpu(void);
void panic(char *fmt, ...) __attribute__((noreturn));
void vpanic(char *fmt, va_list ap) __attribute__((noreturn));
void schedinit(void);
void scheduler(void) __attribute__((noreturn));
void mkproc(byte *data);
void procbegin(void);
void initkstack(Proc *p);
void initstack(Proc *p, u64 entry);
void archrfork(Proc *old, Proc *new);

void yield(void);

// switch.S
void swtch(Registers **from, Registers *to);

// syscall.c
int argfd(int n, int *fd);
int argint(int n, int *ip);
int arglong(int n, long *lp);
int argptr(int n, uintptr *p, usize size);
long argstr(int n, char **pp);

// timer.c
void timerinit(void);
void handletimer(void);

// trap.c
void trapinit(void);

// vm.c
usize allocuvm(Pte *pgmap, usize oldsz, usize newsz);
void clearpteu(Pte *pgmap, void *addr);
Pte *copyuvm(Pte *oldmap, usize sz);
void freeuvm(Pte *pgmap);
void kvmalloc(void);
void loaduvm(Pte *pgmap, char *addr, byte *data, usize sz);
void seginit(void);
Pte *setupkvm(void);
void switchkvm();
void switchuvm(Proc *p);
void checkalign(void *a, int alignment, char *msg, ...);
int pte_flags(Pte entry);
uintptr pte_addr(Pte entry);
int uvmperm(void);
Pte *walkpgmap(Pte *pgmap, void *va, int alloc);
int mappages(Pte *pgmap, void *va, usize size, uintptr pa, int perm);
Pte *allocpgmap(void);
