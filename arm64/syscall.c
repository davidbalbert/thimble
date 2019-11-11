#include "u.h"

#include "arm64.h"
#include "defs.h"

void
syscall(TrapFrame *tf)
{
    cprintf("syscall\n");
}
