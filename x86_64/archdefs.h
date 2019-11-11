typedef struct SyscallFrame SyscallFrame;

// x86_64/syscall.c
long argfd(SyscallFrame *f, int n, int *fd);
long argint(SyscallFrame *f, int n, int *ip);
long arglong(SyscallFrame *f, int n, long *lp);
long argptr(SyscallFrame *f, int n, uintptr *p, usize size);
long argstr(SyscallFrame *f, int n, char **pp);

// x86_64/syscallasm.S
void sysinit(void);
