// libc routines shared between kernel space and userspace
#include "u.h"

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

// GCC wants memcpy for *foo = *bar. We'll just use memmove.
void *
memcpy(void *dst, void *src, usize n)
{
    return memmove(dst, src, n);
}

void *
memset(void *p, int c, usize n)
{
    byte *cp = p;
    usize i;

    for (i = 0; i < n; i++) {
        cp[i] = (byte)c;
    }

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

int
strcmp(char *s1, char *s2)
{
    char *p1, *p2;

    p1 = s1;
    p2 = s2;

    for (;;) {
        if (*p1 == '\0' && *p2 == '\0')
            return 0;
        else if (*p1 == '\0')
            return -1;
        else if (*p2 == '\0')
            return 1;
        else if (*p1 < *p2)
            return -1;
        else if (*p1 > *p2)
            return 1;

        p1++;
        p2++;
    }
}

usize
strlen(char *s)
{
    usize len;

    for (len = 0; *s != '\0'; s++, len++)
        ;

    return len;
}
