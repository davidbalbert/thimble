#include "u.h"
#include "libc.h"
#include "syscall.h"

long syscall();

long
hello(int a, int b, int c, int d, int e, int f)
{
    return syscall(SYS_HELLO, a, b, c, d, e, f);
}

long
goodbye(int a, int b, int c, int d, int e, int f)
{
    return syscall(SYS_GOODBYE, a, b, c, d, e, f);
}

void
print(char *s)
{
    syscall(SYS_PRINT, s);
}

void
printlong(long l)
{
    syscall(SYS_PRINTLONG, l);
}

int
open(char *file, int omode)
{
    return (int)syscall(SYS_OPEN, file, omode);
}

long
read(int fd, void *buf, usize nbytes)
{
    return syscall(SYS_READ, fd, buf, nbytes);
}
