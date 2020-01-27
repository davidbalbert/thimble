#include "u.h"
#include "libc.h"

int
main(void)
{
    char *s = "hello task2!\n";
    int fd = open("/dev/cons", OWRITE);

    write(fd, s, strlen(s));

    for (;;) ;
}
