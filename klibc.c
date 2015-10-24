// libc routines shared between kernel space and userspace
#include "types.h"

void *
memset(void *p, int c, size_t len)
{
    uchar *a = p;
    for (; a < (uchar *)p + len; a++)
        *a = c;

    return p;
}
