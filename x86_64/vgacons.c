// Simple vga console driver. Can be used by the kernel and the stage 2 bootloader.
// No external dependencies besides a void panic(char *) function.
//
// N.b. The cursor position is not shared between the bootloader and kernel.
// Printing from the kernel will write over anything printed from the
// bootloader.

#include "u.h"

#include "console.h"
#include "mem.h"
#include "x86.h"

// Function declarations
// Because console.c is used from both the bootloader and the
// kernel we don't know whether to include defs.h or bootdefs.h.
// For now, we'll just put all external function declarations here.
// I'm not sure if I'll like this, but it's better than putting
// them all throughout the file.

void panic(char *s) __attribute__((noreturn));
int isdigit(int c);
long strtol(char *s, char **endptr, int base);

#define COLOR 0x0700
#define SPACE (COLOR | ' ')

#define COLS 80
#define ROWS 25

#define CSIZE (COLS*ROWS)

// TODO: we should have a console lock

static volatile ushort *vmem = (ushort *)P2V(0xB8000);
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

static void
vcclear(void)
{
    int i;

    for (i = 0; i < CSIZE; i++) {
        vmem[i] = SPACE;
    }

    pos = 0;
    updatecursor();
}

static void
vcscroll(void)
{
    int i;

    for (i = 0; i < CSIZE - COLS; i++)
        vmem[i] = vmem[i + COLS];

    for (i = CSIZE - COLS; i < CSIZE; i++)
        vmem[i] = SPACE;

    pos = CSIZE - COLS;
}

static void vcputs0(char *s);

static void
vcputc0(uchar c)
{
    if (c == '\0') // todo: include all other non-printable characters
        return;
    else if (c == '\n') {
        pos += COLS - pos%COLS;

        if (pos >= CSIZE)
            vcscroll();

        return;
    } else if (c == '\t') {
        vcputs0("        ");
        return;
    } else if (c == '\b') {
        if (pos % COLS != 0)
            vmem[--pos] = SPACE;
        return;
    }

    if (pos >= CSIZE)
        vcscroll();

    vmem[pos++] = COLOR | c;
}

static void
vcputc(uchar c)
{
    vcputc0(c);
    updatecursor();
}

static void
vcputs0(char *s)
{
    for (; *s; s++)
        vcputc0(*s);
}

static void
vcputs(char *s)
{
    vcputs0(s);
    updatecursor();
}

void
vcwrite(char *buf, usize nbytes)
{
    for (; nbytes > 0; nbytes--, buf++)
        vcputc0(*buf);
    updatecursor();
}

static Console vga_console0 = {
    .clear = vcclear,
    .puts = vcputs,
    .putc = vcputc,
};

Console *vga_console = &vga_console0;
