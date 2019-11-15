#include "u.h"

#include "bcm2837.h"
#include "gpio.h"

void
sdinit(void)
{
    // gpio47 - SD card detect
    gpio_setfunc(GPIO_47, GPIO_IN);
    gpio_setpull(GPIO_47, GPIO_PULL_DOWN);
    gpio_setdetect(GPIO_47, GPIO_EVENT_HIGH);
}
