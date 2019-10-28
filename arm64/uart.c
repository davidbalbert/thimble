#include "u.h"

#include "console.h"
#include "mem.h"

#include "bcm2837.h"

#define GPFSEL0         ((volatile u32 *)(PBASE+0x00200000))
#define GPFSEL1         ((volatile u32 *)(PBASE+0x00200004))
#define GPFSEL2         ((volatile u32 *)(PBASE+0x00200008))
#define GPFSEL3         ((volatile u32 *)(PBASE+0x0020000C))
#define GPFSEL4         ((volatile u32 *)(PBASE+0x00200010))
#define GPFSEL5         ((volatile u32 *)(PBASE+0x00200014))
#define GPSET0          ((volatile u32 *)(PBASE+0x0020001C))
#define GPSET1          ((volatile u32 *)(PBASE+0x00200020))
#define GPCLR0          ((volatile u32 *)(PBASE+0x00200028))
#define GPLEV0          ((volatile u32 *)(PBASE+0x00200034))
#define GPLEV1          ((volatile u32 *)(PBASE+0x00200038))
#define GPEDS0          ((volatile u32 *)(PBASE+0x00200040))
#define GPEDS1          ((volatile u32 *)(PBASE+0x00200044))
#define GPHEN0          ((volatile u32 *)(PBASE+0x00200064))
#define GPHEN1          ((volatile u32 *)(PBASE+0x00200068))
#define GPPUD           ((volatile u32 *)(PBASE+0x00200094))
#define GPPUDCLK0       ((volatile u32 *)(PBASE+0x00200098))
#define GPPUDCLK1       ((volatile u32 *)(PBASE+0x0020009C))

/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile u32 *)(PBASE+0x00215004))
#define AUX_MU_IO       ((volatile u32 *)(PBASE+0x00215040))
#define AUX_MU_IER      ((volatile u32 *)(PBASE+0x00215044))
#define AUX_MU_IIR      ((volatile u32 *)(PBASE+0x00215048))
#define AUX_MU_LCR      ((volatile u32 *)(PBASE+0x0021504C))
#define AUX_MU_MCR      ((volatile u32 *)(PBASE+0x00215050))
#define AUX_MU_LSR      ((volatile u32 *)(PBASE+0x00215054))
#define AUX_MU_MSR      ((volatile u32 *)(PBASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile u32 *)(PBASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile u32 *)(PBASE+0x00215060))
#define AUX_MU_STAT     ((volatile u32 *)(PBASE+0x00215064))
#define AUX_MU_BAUD     ((volatile u32 *)(PBASE+0x00215068))

#define AUX_ENABLE_UART 1
#define LCR_8_BIT 3

#define IIR_RX_FIFO_CLEAR (1 << 2)
#define IIR_TX_FIFO_CLEAR (1 << 1)

#define CNTL_RX_ENABLE 1
#define CNTL_TX_ENABLE (1 << 1)

#define LSR_CAN_TX (1 << 5)
#define LSR_CAN_RX 1

// TODO: add write barriers. See "1.3 Peripheral access precautions for correct
// memory ordering" in "BCM2837 ARM Peripherals"

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void
uart_init(void)
{
    u32 r;

    /* initialize UART */
    *AUX_ENABLE |= AUX_ENABLE_UART;
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = LCR_8_BIT;
    //*AUX_MU_MCR = 0;
    *AUX_MU_IER = 0;
    *AUX_MU_IIR |= IIR_RX_FIFO_CLEAR | IIR_TX_FIFO_CLEAR;
    *AUX_MU_BAUD = 270; // 115200 baud

    /* map UART1 to GPIO pins */
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(2<<12)|(2<<15);    // alt5
    *GPFSEL1 = r;

    *GPPUD = 0;            // enable pins 14 and 15

    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }

    *GPPUDCLK0 = 0; // flush GPIO setup

    *AUX_MU_CNTL = CNTL_TX_ENABLE | CNTL_RX_ENABLE;
}

/**
 * Send a character
 */
void
uart_putc(uchar c) {
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
    uchar c;

    for (;;) {
        if (*AUX_MU_LSR & LSR_CAN_RX) {
            break;
        }
    }

    c = (uchar)(*AUX_MU_IO);

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

static Console uart_console0 = {
    .puts = uart_puts,
    .putc = uart_putc,
    .clear = uart_clear,
};

Console *uart_console = &uart_console0;
