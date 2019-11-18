#include "u.h"

#include "defs.h"
#include "mem.h"

#include "bcm2837.h"
#include "gpio.h"

#define EMMC_BASE (PBASE+0x300000)

#define EMMC_ARG2           ((volatile unsigned int*)(EMMC_BASE+0x00))
#define EMMC_BLKSIZECNT     ((volatile unsigned int*)(EMMC_BASE+0x04))
#define EMMC_ARG1           ((volatile unsigned int*)(EMMC_BASE+0x08))
#define EMMC_CMDTM          ((volatile unsigned int*)(EMMC_BASE+0x0C))
#define EMMC_RESP0          ((volatile unsigned int*)(EMMC_BASE+0x10))
#define EMMC_RESP1          ((volatile unsigned int*)(EMMC_BASE+0x14))
#define EMMC_RESP2          ((volatile unsigned int*)(EMMC_BASE+0x18))
#define EMMC_RESP3          ((volatile unsigned int*)(EMMC_BASE+0x1C))
#define EMMC_DATA           ((volatile unsigned int*)(EMMC_BASE+0x20))
#define EMMC_STATUS         ((volatile unsigned int*)(EMMC_BASE+0x24))
#define EMMC_CONTROL0       ((volatile unsigned int*)(EMMC_BASE+0x28))
#define EMMC_CONTROL1       ((volatile unsigned int*)(EMMC_BASE+0x2C))
#define EMMC_INTERRUPT      ((volatile unsigned int*)(EMMC_BASE+0x30))
#define EMMC_INT_MASK       ((volatile unsigned int*)(EMMC_BASE+0x34))
#define EMMC_INT_EN         ((volatile unsigned int*)(EMMC_BASE+0x38))
#define EMMC_CONTROL2       ((volatile unsigned int*)(EMMC_BASE+0x3C))
#define EMMC_SLOTISR_VER    ((volatile unsigned int*)(EMMC_BASE+0xFC))

// gpio47 - SD card detect
// gpio48 - SD card clock
// gpio49 - SD card command
// gpio50 - dat0
// gpio51 - dat1
// gpio52 - dat2
// gpio53 - dat3

void
sdinit(void)
{
    u64 all = GPIO_47|GPIO_48|GPIO_49|GPIO_50|GPIO_51|GPIO_52|GPIO_53;

    gpio_setpull(all, GPIO_PULL_DOWN);
    gpio_setfunc(all & ~GPIO_47, GPIO_ALT3);
    gpio_setfunc(GPIO_47, GPIO_IN);
    gpio_setdetect(GPIO_47, GPIO_EVENT_HIGH);

    cprintf("sd host controller version: %d\n", (*EMMC_SLOTISR_VER >> 16) & 0xFF);
}
