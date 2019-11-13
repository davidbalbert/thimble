typedef struct TrapFrame TrapFrame;

// arm64/syscall.c
void syscall(TrapFrame *tf);
int argfd(int n, int *fd);
int argint(int n, int *ip);
int arglong(int n, long *lp);
int argptr(int n, uintptr *p, usize size);
long argstr(int n, char **pp);

// proc.c
void start(void (*f)(void));
