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

void gpio_setfunc(u64 pins, GpioAlt alt);
void gpio_setpull(u64 pins, GpioPull pull);

#define GPIO_0 (1 << 0)
#define GPIO_1 (1 << 1)
#define GPIO_2 (1 << 2)
#define GPIO_3 (1 << 3)
#define GPIO_4 (1 << 4)
#define GPIO_5 (1 << 5)
#define GPIO_6 (1 << 6)
#define GPIO_7 (1 << 7)
#define GPIO_8 (1 << 8)
#define GPIO_9 (1 << 9)
#define GPIO_10 (1 << 10)
#define GPIO_11 (1 << 11)
#define GPIO_12 (1 << 12)
#define GPIO_13 (1 << 13)
#define GPIO_14 (1 << 14)
#define GPIO_15 (1 << 15)
#define GPIO_16 (1 << 16)
#define GPIO_17 (1 << 17)
#define GPIO_18 (1 << 18)
#define GPIO_19 (1 << 19)
#define GPIO_20 (1 << 20)
#define GPIO_21 (1 << 21)
#define GPIO_22 (1 << 22)
#define GPIO_23 (1 << 23)
#define GPIO_24 (1 << 24)
#define GPIO_25 (1 << 25)
#define GPIO_26 (1 << 26)
#define GPIO_27 (1 << 27)
#define GPIO_28 (1 << 28)
#define GPIO_29 (1 << 29)
#define GPIO_30 (1 << 30)
#define GPIO_31 (1 << 31)
#define GPIO_32 (1 << 32)
#define GPIO_33 (1 << 33)
#define GPIO_34 (1 << 34)
#define GPIO_35 (1 << 35)
#define GPIO_36 (1 << 36)
#define GPIO_37 (1 << 37)
#define GPIO_38 (1 << 38)
#define GPIO_39 (1 << 39)
#define GPIO_40 (1 << 40)
#define GPIO_41 (1 << 41)
#define GPIO_42 (1 << 42)
#define GPIO_43 (1 << 43)
#define GPIO_44 (1 << 44)
#define GPIO_45 (1 << 45)
#define GPIO_46 (1 << 46)
#define GPIO_47 (1 << 47)
#define GPIO_48 (1 << 48)
#define GPIO_49 (1 << 49)
#define GPIO_50 (1 << 50)
#define GPIO_51 (1 << 51)
#define GPIO_52 (1 << 52)
#define GPIO_53 (1 << 53)
