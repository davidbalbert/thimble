#include "u.h"
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
        hello(1, 2, 3, 4, 5, 6);
        wastetime();
    }
}
