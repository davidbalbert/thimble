#include "types.h"

#include "x86.h"

static ushort pos = 0;

static ushort *vmem = (ushort *)0xB8000;

#define COLOR 0x07
#define SPACE (COLOR << 8 | 0x20)

#define COLS 80
#define ROWS 25

#define CSIZE COLS*ROWS

void
cclear(void)
{
    int i;

    for (i = 0; i < CSIZE; i++) {
        vmem[i] = SPACE;
    }

    pos = 0;
}

void
cscroll(void)
{
    int i;

    for (i = 0; i < CSIZE - COLS; i++)
        vmem[i] = vmem[i + COLS];

    for (i = CSIZE - COLS; i < CSIZE; i++)
        vmem[i] = SPACE;

    pos = CSIZE - COLS;
}

void
cputc(uchar c)
{
    if (c == '\n') {
        pos += 80 - pos%80;

        if (pos >= CSIZE)
            cscroll();

        return;
    }

    if (pos >= CSIZE)
        cscroll();


    vmem[pos++] = COLOR<<8 | c;
}

void
cputs(char *s)
{
    for (; *s; s++)
        cputc(*s);
}
