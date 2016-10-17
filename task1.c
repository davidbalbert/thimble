#include "u.h"
#include "libc.h"

void
wastetime(void)
{
    ulong j;
    for (j = 0; j < 100000000; j++)
        ;
}

// placeholder until I port printf/print
void
ltoa(long n, char *buf)
{
    char *numbers = "0123456789";
    char tmp[21]; // max number of digits for a decimal long, optional sign, and null byte.
    ulong n2;
    int i, j;

    if (n < 0)
        n2 = -n;
    else
        n2 = n;

    i = 0;

    do {
        tmp[i++] = numbers[n2 % 10];
        n2 /= 10;
    } while (n2 != 0);

    if (n < 0)
        tmp[i++] = '-';

    for (j = 0, i -= 1; i >= 0; i--, j++)
        buf[j] = tmp[i];

    buf[j] = '\0';
}

int
main(void)
{
    int fd;
    char buf[21];
    long l = 0;
    char *label = "task1: ";

    fd = open("/dev/cons", OWRITE);

    for(;;) {
        write(fd, label, strlen(label));
        ltoa(l++, buf);
        write(fd, buf, strlen(buf));
        write(fd, "\n", 1);
        wastetime();
    }
}
