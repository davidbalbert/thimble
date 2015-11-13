// Simple console driver. Can be used by the kernel and the stage 2 bootloader.
// No external dependencies besides a void panic(char *) function.
//
// N.b. The cursor position is not shared between the bootloader and kernel.
// Printing from the kernel will write over anything printed from the
// bootloader.

#include "types.h"

#include "mem.h"
#include "x86.h"

#include <stdarg.h>

#define COLOR 0x0700
#define SPACE (COLOR | ' ')

#define COLS 80
#define ROWS 25

#define CSIZE COLS*ROWS

static ushort *vmem = (ushort *)P2V(0xB8000);
static ushort pos = 0;

static void
updatecursor(void)
{
    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, pos);
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, pos>>8);
}

void
cclear(void)
{
    int i;

    for (i = 0; i < CSIZE; i++) {
        vmem[i] = SPACE;
    }

    pos = 0;
    updatecursor();
}

static void
cscroll(void)
{
    int i;

    for (i = 0; i < CSIZE - COLS; i++)
        vmem[i] = vmem[i + COLS];

    for (i = CSIZE - COLS; i < CSIZE; i++)
        vmem[i] = SPACE;

    pos = CSIZE - COLS;
}

static void cputs0(char *s);

static void
cputc0(uchar c)
{
    if (c == '\n') {
        pos += COLS - pos%COLS;

        if (pos >= CSIZE)
            cscroll();

        return;
    } else if (c == '\t') {
        cputs0("        ");
        return;
    } else if (c == '\b') {
        if (pos % COLS != 0)
            vmem[--pos] = SPACE;
        return;
    }

    if (pos >= CSIZE)
        cscroll();

    vmem[pos++] = COLOR | c;
}

void
cputc(uchar c)
{
    cputc0(c);
    updatecursor();
}

static void
cputs0(char *s)
{
    for (; *s; s++)
        cputc0(*s);
}

void
cputs(char *s)
{
    cputs0(s);
    updatecursor();
}

void
printint(long n, uchar base, uchar sign)
{
    char *numbers = "0123456789abcdef";
    char buf[24];
    int i = 0;
    ulong n2;

    if (sign && (sign = n < 0))
        n2 = -n;
    else
        n2 = n;

    do {
        buf[i++] = numbers[n2 % base];
        n2 /= base;
    } while (n2 != 0);

    if (base == 16) {
        buf[i++] = 'x';
        buf[i++] = '0';
    } else if (base == 8) {
        buf[i++] = '0';
    }

    if (sign)
        buf[i++] = '-';

    for (i -= 1; i > -1; i--)
        cputc0(buf[i]);
}

void panic(char *s);

void
cprintf(char *fmt, ...)
{
    va_list ap;
    char c;
    char *s;

    if (fmt == 0)
        panic("null fmt");

    va_start(ap, fmt);

    for (; (c = *fmt); fmt++) {
        if (c != '%') {
            cputc0(c);
            continue;
        }

        c = *(++fmt);

        if (c == 0)
            break;

        switch (c) {
            case '%':
                cputc0('%');
                break;
            case 'c':
                cputc0((char)va_arg(ap, int));
            case 's':
                s = va_arg(ap, char *);
                if (s == 0)
                    s = "(null)";
                for (; *s; s++)
                    cputc0(*s);
                break;
            case 'd':
                printint(va_arg(ap, int), 10, 1);
                break;
            case 'o':
                printint(va_arg(ap, int), 8, 0);
                break;
            case 'l':
                printint(va_arg(ap, long), 10, 1);
                break;
            case 'u':
                printint(va_arg(ap, long), 10, 0);
                break;
            case 'p':
            case 'x':
                printint(va_arg(ap, long), 16, 0);
                break;
            default:
                cputc0('%');
                cputc0(c);
                break;
        }
    }

    va_end(ap);
    updatecursor();
}
