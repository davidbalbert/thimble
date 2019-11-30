#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "gpio.h"
#include "mem.h"

#include "bcm2837.h"

#define GPIO_BASE (PBASE+0x200000)

#define GPFSEL0         ((volatile u32 *)(GPIO_BASE+0x00)) // function select (6 registers)

#define GPSET0          ((volatile u32 *)(GPIO_BASE+0x1C)) // pin output set
#define GPSET1          ((volatile u32 *)(GPIO_BASE+0x20))
#define GPCLR0          ((volatile u32 *)(GPIO_BASE+0x28)) // pin output clear
#define GPCLR1          ((volatile u32 *)(GPIO_BASE+0x2C))
#define GPLEV0          ((volatile u32 *)(GPIO_BASE+0x34)) // pin level
#define GPLEV1          ((volatile u32 *)(GPIO_BASE+0x38))
#define GPEDS0          ((volatile u32 *)(GPIO_BASE+0x40)) // event detect
#define GPEDS1          ((volatile u32 *)(GPIO_BASE+0x44))
#define GPREN0          ((volatile u32 *)(GPIO_BASE+0x4C)) // rising edge detect enable
#define GPREN1          ((volatile u32 *)(GPIO_BASE+0x50))
#define GPFEN0          ((volatile u32 *)(GPIO_BASE+0x58)) // falling edge detect enable
#define GPFEN1          ((volatile u32 *)(GPIO_BASE+0x5C))
#define GPHEN0          ((volatile u32 *)(GPIO_BASE+0x64)) // high detect enable
#define GPHEN1          ((volatile u32 *)(GPIO_BASE+0x68))
#define GPLEN0          ((volatile u32 *)(GPIO_BASE+0x70)) // low detect enable
#define GPLEN1          ((volatile u32 *)(GPIO_BASE+0x74))
#define GPAREN0         ((volatile u32 *)(GPIO_BASE+0x7C)) // async rising detect enable
#define GPAREN1         ((volatile u32 *)(GPIO_BASE+0x80))
#define GPAFEN0         ((volatile u32 *)(GPIO_BASE+0x88)) // async falling detect enable
#define GPAFEN1         ((volatile u32 *)(GPIO_BASE+0x8C))
#define GPPUD           ((volatile u32 *)(GPIO_BASE+0x94)) // pull-up/down enable
#define GPPUDCLK0       ((volatile u32 *)(GPIO_BASE+0x98)) // pull-up/down clock (low)
#define GPPUDCLK1       ((volatile u32 *)(GPIO_BASE+0x9C)) // pull-up/down clock (high)

static void
gpio_setfunc0(u64 pin, GpioAlt alt)
{
    if (pin < 0 || pin > 53)
        panic("gpio_setfunc0 - invalid pin");

    if (alt < 0 || alt > 7)
        panic("gpio_setfunc0 - invalid alt");

    dmb();

    int offset = pin / 10;
    int shift = (pin%10)*3;

    u32 r = *(GPFSEL0+offset);

    r &= ~(7 << shift);  // clear the existing value for pin
    r |= (alt << shift); // and set the new value

    *(GPFSEL0+offset) = r;
}

void
gpio_setfunc(u64 pins, GpioAlt alt)
{
    int pin;

    while (pins) {
        pin = ctz(pins);
        gpio_setfunc0(pin, alt);
        pins &= ~(1l << pin);
    }
}

void
gpio_setpull(u64 pins, GpioPull pull)
{
    int i;
    u32 low, high;

    dmb();

    if (pull < 0 || pull > 2)
        panic("gpio_setpull - invalid pull");

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

void gpio_setdetect(u64 pins, GpioEvent event)
{
    u32 low, high;

    dmb();

    low = pins & 0xFFFFFFFF;
    high = pins >> 32;

    switch (event) {
        case GPIO_EVENT_RISING:
            if (low) *GPREN0 |= low;
            if (high) *GPREN1 |= high;
            break;
        case GPIO_EVENT_FALLING:
            if (low) *GPFEN0 |= low;
            if (high) *GPFEN1 |= high;
            break;
        case GPIO_EVENT_HIGH:
            if (low) *GPHEN0 |= low;
            if (high) *GPHEN1 |= high;
            break;
        case GPIO_EVENT_LOW:
            if (low) *GPLEN0 |= low;
            if (high) *GPLEN1 |= high;
            break;
        case GPIO_EVENT_ASYNC_RISING:
            if (low) *GPAREN0 |= low;
            if (high) *GPAREN1 |= high;
            break;
        case GPIO_EVENT_ASYNC_FALLING:
            if (low) *GPAFEN0 |= low;
            if (high) *GPAFEN1 |= high;
            break;
        default:
            panic("gpio_setdetect - invalid event");
    }
}
