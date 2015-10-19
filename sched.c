#include "types.h"

#include "common.h"
#include "x86.h"

void
scheduler(void)
{
    for (;;)
        hlt();
}
