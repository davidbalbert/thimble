typedef enum {
    GPIO_IN   = 0b000,
    GPIO_OUT  = 0b001,
    GPIO_ALT0 = 0b100,
    GPIO_ALT1 = 0b101,
    GPIO_ALT2 = 0b110,
    GPIO_ALT3 = 0b111,
    GPIO_ALT4 = 0b011,
    GPIO_ALT5 = 0b010,
} GpioAlt;

typedef enum {
    GPIO_PULL_NONE = 0,
    GPIO_PULL_UP   = 1,
    GPIO_PULL_DOWN = 2,
} GpioPull;

typedef enum {
    GPIO_EVENT_RISING,
    GPIO_EVENT_FALLING,
    GPIO_EVENT_HIGH,
    GPIO_EVENT_LOW,
    GPIO_EVENT_ASYNC_RISING,
    GPIO_EVENT_ASYNC_FALLING,
} GpioEvent;

void gpio_setfunc(u64 pins, GpioAlt alt);
void gpio_setpull(u64 pins, GpioPull pull);
void gpio_setdetect(u64 pins, GpioEvent event);

#define GPIO_0 (1l << 0)
#define GPIO_1 (1l << 1)
#define GPIO_2 (1l << 2)
#define GPIO_3 (1l << 3)
#define GPIO_4 (1l << 4)
#define GPIO_5 (1l << 5)
#define GPIO_6 (1l << 6)
#define GPIO_7 (1l << 7)
#define GPIO_8 (1l << 8)
#define GPIO_9 (1l << 9)
#define GPIO_10 (1l << 10)
#define GPIO_11 (1l << 11)
#define GPIO_12 (1l << 12)
#define GPIO_13 (1l << 13)
#define GPIO_14 (1l << 14)
#define GPIO_15 (1l << 15)
#define GPIO_16 (1l << 16)
#define GPIO_17 (1l << 17)
#define GPIO_18 (1l << 18)
#define GPIO_19 (1l << 19)
#define GPIO_20 (1l << 20)
#define GPIO_21 (1l << 21)
#define GPIO_22 (1l << 22)
#define GPIO_23 (1l << 23)
#define GPIO_24 (1l << 24)
#define GPIO_25 (1l << 25)
#define GPIO_26 (1l << 26)
#define GPIO_27 (1l << 27)
#define GPIO_28 (1l << 28)
#define GPIO_29 (1l << 29)
#define GPIO_30 (1l << 30)
#define GPIO_31 (1l << 31)
#define GPIO_32 (1l << 32)
#define GPIO_33 (1l << 33)
#define GPIO_34 (1l << 34)
#define GPIO_35 (1l << 35)
#define GPIO_36 (1l << 36)
#define GPIO_37 (1l << 37)
#define GPIO_38 (1l << 38)
#define GPIO_39 (1l << 39)
#define GPIO_40 (1l << 40)
#define GPIO_41 (1l << 41)
#define GPIO_42 (1l << 42)
#define GPIO_43 (1l << 43)
#define GPIO_44 (1l << 44)
#define GPIO_45 (1l << 45)
#define GPIO_46 (1l << 46)
#define GPIO_47 (1l << 47)
#define GPIO_48 (1l << 48)
#define GPIO_49 (1l << 49)
#define GPIO_50 (1l << 50)
#define GPIO_51 (1l << 51)
#define GPIO_52 (1l << 52)
#define GPIO_53 (1l << 53)
