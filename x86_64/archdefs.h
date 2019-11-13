// x86_64/syscall.c
long argfd(int n, int *fd);
long argint(int n, int *ip);
long arglong(int n, long *lp);
long argptr(int n, uintptr *p, usize size);
long argstr(int n, char **pp);

// x86_64/syscallasm.S
void sysinit(void);
