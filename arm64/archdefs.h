typedef struct TrapFrame TrapFrame;

// arm64/syscall.c
void syscall(TrapFrame *tf);
long argfd(int n, int *fd);
long argint(int n, int *ip);
long arglong(int n, long *lp);
long argptr(int n, uintptr *p, usize size);
long argstr(int n, char **pp);

// proc.c
void start(void (*f)(void));
