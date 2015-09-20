#include "types.h"

#include "x86.h"

static ushort pos = 0;

static ushort *vmem = (ushort *)0xB8000;

#define COLOR 0x07
#define SPACE (COLOR << 8 | 0x20)

#define COLS 80
#define ROWS 25

#define CSIZE COLS*ROWS

static void
updatecursor()
{
    // TODO: read base port from BIOS data area

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

void
cscroll(void)
{
    int i;

    for (i = 0; i < CSIZE - COLS; i++)
        vmem[i] = vmem[i + COLS];

    for (i = CSIZE - COLS; i < CSIZE; i++)
        vmem[i] = SPACE;

    pos = CSIZE - COLS;
    updatecursor();
}

void
cputc(uchar c)
{
    if (c == '\n') {
        pos += 80 - pos%80;

        if (pos >= CSIZE)
            cscroll();

        updatecursor();
        return;
    }

    if (pos >= CSIZE)
        cscroll();

    vmem[pos++] = COLOR<<8 | c;
    updatecursor();
}

void
cputs(char *s)
{
    for (; *s; s++)
        cputc(*s);
}
