// libc routines shared between kernel space and userspace
#include "types.h"
#include "x86.h"

void *
memset(void *p, int c, usize len)
{
    stosb(p, c, len);
    return p;
}

void *
memzero(void *p, usize len)
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
