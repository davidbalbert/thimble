#include "u.h"
#include "libc.h"
#include "syscall.h"

long syscall();

int
open(char *file, int omode)
{
    return (int)syscall(SYS_OPEN, file, omode);
}

int
close(int fd)
{
    return (int)syscall(SYS_CLOSE, fd);
}

long
read(int fd, void *buf, usize nbytes)
{
    return syscall(SYS_READ, fd, buf, nbytes);
}

long
write(int fd, void *buf, usize nbytes)
{
    return syscall(SYS_WRITE, fd, buf, nbytes);
}
