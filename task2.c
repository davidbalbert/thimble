#include "types.h"
#include "libc.h"

void
wastetime(void)
{
    ulong j;
    for (j = 0; j < 100000000; j++)
        ;
}

int
main(void)
{
    for(;;) {
        goodbye(6, 5, 4, 3, 2, 1);
        wastetime();
    }
}
