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

int
isprint(int c)
{
    return c >= ' ' && c <= '~';
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

// Writes r to s, encoded as UTF-8. Returns number of bytes written.
int
runetochar(char *s, Rune *r)
{
    Rune c = *r;

    if (c >= 0x110000) {
        c = 0xFFFD; // replacement character
    }

    if (c < 0x80) {
        *s = c & 0x7F;
        return 1;
    } else if (c < 0x800) {
        *s++ = (0b110<<5) | (c >> 6);
        *s   = (0b10<<6)  | (c & 0x3F);
        return 2;
    } else if (c < 0x10000) {
        *s++ = (0b1110<<4) | (c >> 12);
        *s++ = (0b10<<6)   | ((c >> 6) & 0x3F);
        *s   = (0b10<<6)   | (c & 0x3F);
        return 3;
    } else {
        *s++ = (0b11110<<3) | (c >> 18);
        *s++ = (0b10<<6)    | ((c >> 12) & 0x3F);
        *s++ = (0b10<<6)    | ((c >> 6) & 0x3F);
        *s   = (0b10<<6)    | (c & 0x3F);
        return 4;
    }
}

int
runelen(Rune r)
{
    if (r < 0x80) {
        return 1;
    } else if (r < 0x800) {
        return 2;
    } else if (r < 0x10000) {
        return 3;
    } else if (r < 0x110000){
        return 4;
    } else {
        return 3; // larger runes are replaced with the replacement character
    }
}
