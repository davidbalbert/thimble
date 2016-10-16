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

long
print(char *s)
{
    return syscall(SYS_PRINT, s);
}

int
open(char *file, int omode)
{
    return syscall(SYS_OPEN, file, omode);
}
