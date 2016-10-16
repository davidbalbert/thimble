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
    usize nbytes;

    fd = open("/hello.txt", OREAD);

    if (fd < 0) {
        print("couldn't open /hello.txt\n");
        for (;;)
            ;
    }

    if ((nbytes = read(fd, buf, sizeof(buf) - 1)) < 0) {
        print("couldn't read /hello.txt\n");
        for (;;)
            ;
    }

    buf[nbytes] = '\0';
    print(buf);

    for(;;) {
        hello(1, 2, 3, 4, 5, 6);
        wastetime();
    }
}
