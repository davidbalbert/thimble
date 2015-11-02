#include "types.h"

#include "common.h"

int
syscall(int num)
{
    cprintf("syscall(%d)\n", num);
    return 0;
}
