#include "u.h"
#include "libc.h"

void
wastetime(void)
{
    ulong j;
    for (j = 0; j < 100000000; j++)
        ;
}

void
main(void)
{
    //int i = 0;

    for(;;) {
        //printint(i++);
        printhello();
        wastetime();
    }
}
