#include "arm64.h"

void
halt(void)
{
    for (;;) {
        wfi();
    }
}
