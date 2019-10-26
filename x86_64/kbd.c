#include "u.h"

#include "defs.h"
#include "irq.h"
#include "x86.h"

#define PS2_DATA 0x60

#define RELEASE  0x80 // If the high bit is set, it's a key release

#define LCTRL       (1<<0)
#define RCTRL       (1<<1)
#define LALT        (1<<2)
#define RALT        (1<<3)
#define LMETA       (1<<4)
#define RMETA       (1<<5)
#define LSHIFT      (1<<6)
#define RSHIFT      (1<<7)
#define CAPSLOCK    (1<<8)
#define SCROLLLOCK  (1<<9)
#define NUMLOCK     (1<<10)
#define E0MOD       (1<<11)

#define CTRL    (LCTRL | RCTRL)
#define ALT     (LALT | RALT)
#define SHIFT   (LSHIFT | RSHIFT)
#define META    (LMETA | RMETA)

static uint skip = 0;
static uint mods = 0; // bitmap for modifier keys

#define ESC 0
#define CPSLK 0
#define F 0 // F
#define NLK 0 // Number lock
#define SLK 0 // Scroll lock

static uchar charmap[256] = {
    0  , ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9',
    '4', '5', '6', '1', '2', '3', '0', '.',
};

static uchar shiftmap[256] = {
    0  , ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9',
    '4', '5', '6', '1', '2', '3', '0', '.',
};

/*
static uchar e2map[256] = {
};
*/

/*
static uint togglemap[256] = {
    [0x3A] CAPSLOCK,
    [0x45] NUMLOCK,
    [0x45] SCROLLLOCK,
};
*/

static uint modmap[256] = {
    [0x1D] LCTRL,
    [0x2A] LSHIFT,
    [0x36] RSHIFT,
    [0x38] LALT,
};

static uint e0modmap[256] = {
    [0x1D] RCTRL,
};


void
handlekbd(void)
{
    uchar c;
    uchar byte = inb(PS2_DATA);

    if (skip > 0) {
        skip--;
        return;
    } else if (byte == 0xE1) {
        // Skip pause button: [0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5]
        skip = 5;
        return;
    } else if (byte == 0xE0) {
        mods |= E0MOD;
        return;
    } else if (mods & E0MOD && (byte == 0x2A || byte == 0xB7)) {
        // Skip print screen: [0xE0, 0x2A, 0xE0, 0x37] or [0xE0, 0xB7, 0xE0, 0xAA]
        skip = 2;
        mods &= ~E0MOD;
        return;
    } else if (byte & RELEASE) {
        byte &= ~RELEASE;

        mods ^= (mods & E0MOD ? e0modmap[byte] : modmap[byte]);
        mods &= ~E0MOD;
        return;
    }

    mods |= (mods & E0MOD ? e0modmap[byte] : modmap[byte]);

    if (mods & SHIFT)
        c = shiftmap[byte];
    else if (mods & E0MOD)
        c = 0; // e0map[byte];
    else
        c = charmap[byte];

    if (c)
        cputc(c);
}


void
kbdinit(void)
{
    intenable(IRQ_KBD);
}
