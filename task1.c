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
    char buf[1024];
    char *s = "Hello, /dev/cons!\n";

    fd = open("/dev/cons", OWRITE);

    if (write(123, "hello\n", 6) >= 0) {
        print("shouldn't be able to write to fd 123\n");
        for (;;)
            ;
    }

    if (fd < 0) {
        print("couldn't open /dev/cons\n");
        for (;;)
            ;
    }

    if (write(fd, s, strlen(s)) < 0) {
        print("couldn't write to /dev/cons\n");
        for (;;)
            ;
    }

    if (read(fd, buf, sizeof(buf) - 1) >= 0) {
        print("shouldn't have been able to read /dev/cons open for writing only");
        for (;;)
            ;
    }

    close(fd);

    if (write(fd, "hello\n", 6) >= 0) {
        print("shouldn't be able to write to closed fd\n");
        for (;;)
            ;
    }

    for(;;) {
        hello(1, 2, 3, 4, 5, 6);
        wastetime();
    }
}
