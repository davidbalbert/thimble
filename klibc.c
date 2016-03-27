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

void *
memzero(void *p, size_t len)
{
    return memset(p, 0, len);
}

int
isdigit(int c)
{
    return c >= '0' && c <= '9';
}

// NOTE:
//  - base is ignored
//  - only supports positive numbers
long
strtol(char *s, char **endptr, int base)
{
    long i = 0;

    while (isdigit(*s)) {
        i = i*10 + (*s - '0');
        s++;
    }

    if (endptr) {
        *endptr = s;
    }

    return i;
}
