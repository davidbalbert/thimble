#include "types.h"
#include "libc.h"
#include "syscall.h"

long syscall(long n, long a, long b, long c, long d, long e, long f);

int
hello(int a, int b, int c, int d, int e, int f)
{
    return syscall(SYS_HELLO, a, b, c, d, e, f);
}

int
goodbye(int a, int b, int c, int d, int e, int f)
{
    return syscall(SYS_GOODBYE, a, b, c, d, e, f);
}
