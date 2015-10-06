#include "types.h"

#include "common.h"
#include "irq.h"
#include "x86.h"

#define PS2_DATA 0x60

#define RELEASE  0x80 // If the high bit is set, it's a key release

#define CTRL        1
#define ALT         2
#define META        4
#define SHIFT       8
#define CAPSLOCK    16
#define E0MOD       32

void
kbdinit(void)
{
    picenable(IRQ_KBD);
}

static uint skip = 0;
static uint mods = 0; // bitmap for modifier keys

#define ESC 0
#define BCK 0
#define TAB 0
#define LALT 0
#define RALT 0
#define LCTL 0
#define RCTL 0
#define LSHF 0
#define RSHF 0
#define CPSLK 0
#define F 0 // F
#define NLK 0 // Number lock
#define SLK 0 // Scroll lock

static uchar charmap[256] = {
    0  , ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', BCK,
    TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    LCTL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    LSHF, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', RSHF, '*',
    LALT, ' ', CPSLK, F, F, F, F, F, F, F, F, F, F, NLK, SLK, '7', '8', '9',
    '4', '5', '6', '1', '2', '3', '0', '.',
    [0x57] = F, // F11
    [0x58] = F, // F12
};

void
handlekbd(void)
{
    uchar c;
    uchar byte = inb(PS2_DATA);

    if (skip > 0) {
        skip--;
        cprintf("%x ", byte);
    } else if (byte == 0xE1) {
        cprintf("0xe1 ");
        skip = 5;       // Skip pause button: [0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5]
    } else if (byte == 0xE0) {
        mods |= E0MOD;
    } else if (mods & E0MOD && (byte == 0x2A || byte == 0xB7)) {
        cprintf("printscreen ");
        skip = 2;       // Skip print screen: [0xE0, 0x2A, 0xE0, 0x37] or [0xE0, 0xB7, 0xE0, 0xAA]
        mods &= ~E0MOD;
    } else if (byte & RELEASE) {
        // Skip all key releases
        mods &= ~E0MOD;
    } else if (mods & E0MOD) {
        mods &= ~E0MOD;
        //othermap[byte];
    } else {
        c = charmap[byte];

        if (c != 0)
            cputc(c);
    }
}
