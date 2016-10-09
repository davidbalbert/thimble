// libc routines shared between kernel space and userspace
#include "u.h"
#include "x86.h"

void *
memmove(void *dst, void *src, usize n)
{
    const char *s = src;
    char *d = dst;

    if (s < d && s+n > d) {
        s += n;
        d += n;

        while (n--)
            *--d = *--s;
    } else {
        while (n--)
            *d++ = *s++;
    }

    return dst;
}

void *
memset(void *p, int c, usize n)
{
    stosb(p, c, n);
    return p;
}

void *
memzero(void *p, usize n)
{
    return memset(p, 0, n);
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
