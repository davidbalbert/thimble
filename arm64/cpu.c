#include "u.h"

#include "arm64.h"

void
halt(void)
{
      wfi();
}
