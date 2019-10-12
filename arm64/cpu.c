#include "u.h"

#include "arm64.h"
#include "mem.h"

char bootkstack[KSTACKSIZE] __attribute__ ((aligned (PGSIZE)));

void
halt(void)
{
      wfi();
}
