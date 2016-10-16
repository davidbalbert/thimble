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
    int fd;
    char *s = "Hello, /dev/cons!\n";

    fd = open("/dev/cons", OWRITE);

    if (fd < 0) {
        print("couldn't open /dev/cons\n");
        for (;;)
            ;
    }

    if (write(fd, s, strlen(s)) < 0) {
        print("couldn't write to /dev/cons");
        for (;;)
            ;
    }

    for(;;) {
        hello(1, 2, 3, 4, 5, 6);
        wastetime();
    }
}
