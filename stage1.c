#include "types.h"

#include "mem.h"
#include "x86.h"

#include "bootide.h"

void
bootmain(void)
{
    void *addr;
    void (*stage2start)(void);

    addr = (void *)0x7E00;
    readsects(addr, 1, 1);

    stage2start = (void(*)(void))addr;
    stage2start();
}
