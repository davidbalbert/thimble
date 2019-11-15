#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "gpio.h"
#include "mem.h"

#include "bcm2837.h"

#define GPIO_BASE (PBASE+0x200000)

#define GPFSEL0         ((volatile u32 *)(GPIO_BASE+0x00))

#define GPPUD           ((volatile u32 *)(GPIO_BASE+0x94))
#define GPPUDCLK0       ((volatile u32 *)(GPIO_BASE+0x98))
#define GPPUDCLK1       ((volatile u32 *)(GPIO_BASE+0x9C))

void
gpio_setfunc(u64 pin, GpioAlt alt)
{
    pin = ctz(pin); // switch from bitmask to pin number.

    if (pin < 0 || pin > 53)
        panic("gpiofsel - invalid pin");

    if (alt < 0 || alt > 7)
        panic("gpiofsel - invalid alt");

    int offset = pin / 10;
    int shift = (pin%10)*3;

    u32 r = *(GPFSEL0+offset);

    r &= ~(7 << shift);  // clear the existing value for pin
    r |= (alt << shift); // and set the new value

    *(GPFSEL0+offset) = r;
}

void
gpio_setpull(u64 pins, GpioPull pull)
{
    int i;
    u32 low, high;

    *GPPUD = pull;

    i=150; while(i--) { asm volatile("nop"); }

    low = pins & 0xFFFFFFFF;
    high = pins >> 32;

    if (low) {
        *GPPUDCLK0 = low;
    }
    if (high) {
        *GPPUDCLK1 = high;
    }

    i=150; while(i--) { asm volatile("nop"); }

    if (low) {
        *GPPUDCLK0 = 0;
    }
    if (high) {
        *GPPUDCLK1 = 0;
    }

    *GPPUD = 0;
}
