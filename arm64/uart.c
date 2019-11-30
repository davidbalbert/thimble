#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "console.h"
#include "irq.h"
#include "mem.h"

#include "bcm2837.h"
#include "gpio.h"

#define AUX_BASE (PBASE+0x215000)

/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile u32 *)(AUX_BASE+0x04))
#define AUX_MU_IO       ((volatile u32 *)(AUX_BASE+0x40))
#define AUX_MU_IER      ((volatile u32 *)(AUX_BASE+0x44))
#define AUX_MU_IIR      ((volatile u32 *)(AUX_BASE+0x48))
#define AUX_MU_LCR      ((volatile u32 *)(AUX_BASE+0x4C))
#define AUX_MU_MCR      ((volatile u32 *)(AUX_BASE+0x50))
#define AUX_MU_LSR      ((volatile u32 *)(AUX_BASE+0x54))
#define AUX_MU_MSR      ((volatile u32 *)(AUX_BASE+0x58))
#define AUX_MU_SCRATCH  ((volatile u32 *)(AUX_BASE+0x5C))
#define AUX_MU_CNTL     ((volatile u32 *)(AUX_BASE+0x60))
#define AUX_MU_STAT     ((volatile u32 *)(AUX_BASE+0x64))
#define AUX_MU_BAUD     ((volatile u32 *)(AUX_BASE+0x68))

#define AUX_ENABLE_UART 1
#define LCR_8_BIT 3

#define IIR_RX_FIFO_CLEAR (1 << 2)
#define IIR_TX_FIFO_CLEAR (1 << 1)
#define IIR_RX_READY      (2 << 1)

#define CNTL_RX_ENABLE (1 << 0)
#define CNTL_TX_ENABLE (1 << 1)

#define LSR_CAN_TX (1 << 5)
#define LSR_CAN_RX 1

#define IER_RX_INT 0b11111101; // enable receive interrupt
#define IER_TX_INT 0b11111110; // enable transmit interrupt

// TODO: add write barriers. See "1.3 Peripheral access precautions for correct
// memory ordering" in "BCM2837 ARM Peripherals"

/**
 * Send a character
 */
void
uart_putc(byte c) {
    dmb();

    for (;;) {
        if (*AUX_MU_LSR & LSR_CAN_TX) {
            break;
        }
    }

    *AUX_MU_IO=c;
}

/**
 * Receive a character
 */
char
uart_getc(void) {
    byte c;

    dmb();

    for (;;) {
        if (*AUX_MU_LSR & LSR_CAN_RX) {
            break;
        }
    }

    c = (byte)(*AUX_MU_IO);

    if (c == '\r') {
        c = '\n';
    }

    return c;
}

/**
 * Display a string
 */
void
uart_puts(char *s) {
    while(*s) {
        if(*s == '\n') {
            uart_putc('\r');
        }

        uart_putc(*s++);
    }
}

static void
uart_clear(void)
{
}

void
handleuart(void)
{
    dmb();

    while (*AUX_MU_IIR & IIR_RX_READY) {
        uart_putc(uart_getc());
    }
}

// Set baud rate and characteristics (115200 8N1) and map to GPIO
void
uart_init(void)
{
    dmb();

    /* initialize UART */
    *AUX_ENABLE |= AUX_ENABLE_UART;
    *AUX_MU_IER = 0;
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = LCR_8_BIT;
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = IER_RX_INT;
    *AUX_MU_IIR |= IIR_RX_FIFO_CLEAR | IIR_TX_FIFO_CLEAR;
    *AUX_MU_BAUD = 270; // 115200 baud

    // use gpio14 and gpio15 for UART1
    gpio_setfunc(GPIO_14|GPIO_15, GPIO_ALT5);
    gpio_setpull(GPIO_14|GPIO_15, GPIO_PULL_NONE);

    intenable(IRQ_AUX);
}

static Console uart_console0 = {
    .puts = uart_puts,
    .putc = uart_putc,
    .clear = uart_clear,
};

Console *uart_console = &uart_console0;
