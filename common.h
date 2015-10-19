typedef struct SpinLock SpinLock;
typedef struct Registers Registers;


// console.c
void panic(char *s) __attribute__((noreturn));
void cprintf(char *fmt, ...);
void cclear(void);
void cputs(char *s);
void cputc(uchar c);

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
void schedinit(void);
void scheduler(void) __attribute__((noreturn));
void start(void (*f)(void));
void yield(void);

// switch.S
void swtch(Registers **from, Registers *to);

// trap.c
void trapinit(void);
