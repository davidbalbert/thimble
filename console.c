#include "u.h"

#include "console.h"
#include "defs.h"

// TODO: we should have a console lock

static Console *console;

void
cinit(Console *cons)
{
    console = cons;
}

void
cclear(void)
{
    console->clear();
}

void
cputc(byte c)
{
    console->putc(c);
}

void
cputs(char *s)
{
    console->puts(s);
}

static void
printint(long n, byte base, byte sign, long npad, char padchar)
{
    char *numbers = "0123456789abcdef";
    char buf[65]; // longest representation is binary. 64 chars + a null bite
    int i = 0;
    u64 n2;

    if (sign && (sign = n < 0))
        n2 = -n;
    else
        n2 = n;

    do {
        buf[i++] = numbers[n2 % base];
        n2 /= base;
    } while (n2 != 0);


    if (npad > 0 && sign)
        npad--;

    if (sign && padchar == '0')
        cputc('-');

    for (; i < npad; npad--)
        cputc(padchar);

    if (sign && padchar == ' ')
        cputc('-');

    for (i -= 1; i > -1; i--)
        cputc(buf[i]);
}

void
cvprintf(char *fmt, va_list ap)
{
    char c;
    char *s;

    if (fmt == nil) {
        // equivalent of panic("null fmt");
        cputs("panic: null fmt");
        for (;;) {
            halt();
        }
    }

    for (; (c = *fmt); fmt++) {
        long npad = 0;
        char padchar = ' ';

        if (c != '%') {
            cputc(c);
            continue;
        }

        fmt++;

        // padding character
        if (*fmt == '0') {
            padchar = '0';
            fmt++;
        }

        // min field width
        if (isdigit(*fmt)) {
            npad = strtol(fmt, &fmt, 0);
        }

        c = *fmt;

        if (c == 0)
            break;

        switch (c) {
            case '%':
                cputc('%');
                break;
            case 'c':
                cputc((char)va_arg(ap, int));
            case 's':
                s = va_arg(ap, char *);
                if (s == 0)
                    s = "(null)";
                for (; *s; s++)
                    cputc(*s);
                break;
            case 'd':
                printint(va_arg(ap, int), 10, 1, npad, padchar);
                break;
            case 'o':
                printint(va_arg(ap, int), 8, 0, npad, padchar);
                break;
            case 'l':
                printint(va_arg(ap, long), 10, 1, npad, padchar);
                break;
            case 'u':
                printint(va_arg(ap, long), 10, 0, npad, padchar);
                break;
            case 'p':
            case 'x':
                printint(va_arg(ap, long), 16, 0, npad, padchar);
                break;
            case 'b':
                printint(va_arg(ap, long), 2, 0, npad, padchar);
                break;
            default:
                cputc('%');
                cputc(c);
                break;
        }
    }
}

void
cprintf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    cvprintf(fmt, ap);
    va_end(ap);
}

void
cwrite(char *buf, usize nbytes)
{
    for (; nbytes > 0; nbytes--, buf++) {
        cputc(*buf);
    }
}
