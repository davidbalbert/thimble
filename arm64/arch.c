#include "u.h"

#include "archdefs.h"
#include "lock.h"
#include "sleeplock.h"
#include "bio.h"
#include "defs.h"
#include "uart.h"

void
archinit_early(void)
{
    uart_init();
    cinit(uart_console);
}

void xxd(byte *data, usize len, usize start);

void
archinit(void)
{
    dmainit();
    sdinit();

    byte *data = kalloc();
    if (data == nil) {
      panic("archinit");
    }

    Buf *b = bread(1, 31);

    xxd(b->data, BSIZE, 63*512);

    brelse(b);
}

usize
dumpline(byte **data, usize len)
{
    byte *p = *data;

    for (usize i = 0, l = len; i < 16 && l; i++, l--) {
        cprintf("%02x", p[i]);

        if (i % 2 == 1) {
            cprintf(" ");
        }
    }

    cprintf(" ");

    for (usize i = 0; i < 16 && len; i++, len--) {
        if (isprint(p[i])) {
            cputc(p[i]);
        } else {
            cprintf(".");
        }
    }

    cprintf("\n");

    *data += 16;
    return len;
}

void
xxd(byte *data, usize len, usize start)
{
    usize i = start;

    while (len > 0) {
        cprintf("%08x: ", i);
        len = dumpline(&data, len);
        i += 16;
    }
}
