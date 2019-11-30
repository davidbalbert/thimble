#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "mem.h"

#include "bcm2837.h"
#include "gpio.h"

#define EMMC_BASE (PBASE+0x300000)

#define EMMC_ARG2           ((volatile u32*)(EMMC_BASE+0x00))
#define EMMC_BLKSIZECNT     ((volatile u32*)(EMMC_BASE+0x04))
#define EMMC_ARG1           ((volatile u32*)(EMMC_BASE+0x08))
#define EMMC_CMDTM          ((volatile u32*)(EMMC_BASE+0x0C))
#define EMMC_RESP0          ((volatile u32*)(EMMC_BASE+0x10))
#define EMMC_RESP1          ((volatile u32*)(EMMC_BASE+0x14))
#define EMMC_RESP2          ((volatile u32*)(EMMC_BASE+0x18))
#define EMMC_RESP3          ((volatile u32*)(EMMC_BASE+0x1C))
#define EMMC_DATA           ((volatile u32*)(EMMC_BASE+0x20))
#define EMMC_STATUS         ((volatile u32*)(EMMC_BASE+0x24))
#define EMMC_CONTROL0       ((volatile u32*)(EMMC_BASE+0x28))
#define EMMC_CONTROL1       ((volatile u32*)(EMMC_BASE+0x2C))
#define EMMC_INTERRUPT      ((volatile u32*)(EMMC_BASE+0x30))
#define EMMC_INT_MASK       ((volatile u32*)(EMMC_BASE+0x34))
#define EMMC_INT_EN         ((volatile u32*)(EMMC_BASE+0x38))
#define EMMC_CONTROL2       ((volatile u32*)(EMMC_BASE+0x3C))
#define EMMC_CAPABILITIES1  ((volatile u32*)(EMMC_BASE+0x40))
#define EMMC_CAPABILITIES2  ((volatile u32*)(EMMC_BASE+0x44))
#define EMMC_SLOTISR_VER    ((volatile u32*)(EMMC_BASE+0xFC))

#define CMD_GO_IDLE_STATE       (0 << 24) // CMD0
#define CMD_ALL_SEND_CID        (2 << 24) // CMD2
#define CMD_SEND_RELATIVE_ADDR  (3 << 24) // ...
#define CMD_SELECT_CARD         (7 << 24)
#define CMD_SEND_IF_COND        (8 << 24)
#define CMD_SEND_CSD            (9 << 24)
#define CMD_READ_MULTIPLE_BLOCK (18 << 24)
#define CMD_APP                 (55 << 24)

#define ACMD_SD_SEND_OP_COND   (41 << 24)

// CMDTM
#define TM_BLKCNT_EN    (1 << 1)
#define TM_AUTO_CMD12   (0b01 << 2)
#define TM_CARD_TO_HOST (1 << 4)
#define TM_MULTI_BLOCK  (1 << 5)
#define RSP_NONE        (0b00 << 16)
#define RSP_136         (0b01 << 16)
#define RSP_48          (0b10 << 16)
#define RSP_48_BUSY     (0b11 << 16)
#define CMD_CRCCHK_EN   (1 << 19)
#define CMD_IXCHK_EN    (1 << 20)
#define CMD_ISDATA      (1 << 21)

// CMD8
#define CHECK_PATTERN 0b10101010
#define VHS_27_36     (0b0001 << 8) // 2.7V - 3.6V

// ACMD41
#define OCR_ALL_VOLTAGES (0b111111111 << 15)
#define OCR_HCS  (1 << 30) // Host Capacity Support (1 == SDHC and SDXC supported)
#define OCR_CSS  (1 << 30) // Card Capacity Status (0 == SDSC, 1 == SDHC or SDXC, which are block addressed)
#define OCR_BUSY (1 << 31) // Zero when busy, one when ready

#define CTL1_CLK_INTLEN           (1 << 0)
#define CTL1_CLK_STABLE           (1 << 1)
#define CTL1_CLK_EN               (1 << 2)
#define CTL1_CLK_GENSEL           (1 << 5)
#define CTL1_DATA_TOUNIT_DISABLED (0b1111 << 16)
#define CTL1_SRST_HC              (1 << 24)

#define INT_ERR (1 << 15)
#define INT_ALL 0x17FF137

struct SdCard {
    int sdhc; // high capacity or extended capacity;
    u32 rca;
};
typedef struct SdCard SdCard;

SdCard card;

// Send command. Non-zero on error.
static int
sendcmd(int cmd, int flags, int arg)
{
    *EMMC_INTERRUPT = INT_ALL;
    *EMMC_ARG1 = arg;
    *EMMC_CMDTM = cmd | flags | CMD_CRCCHK_EN | CMD_IXCHK_EN;

    while (*EMMC_INTERRUPT == 0);

    return *EMMC_INTERRUPT & INT_ERR;
}

void
sdread(byte *addr, u64 lba, u16 count)
{
    if (!card.sdhc) {
        lba *= 512; // SDSC cards are byte addressed
    }

    dmb();

    *EMMC_BLKSIZECNT = (count<<16) | 512; // 512 byte blocks;

    sendcmd(CMD_READ_MULTIPLE_BLOCK,
            TM_BLKCNT_EN | TM_AUTO_CMD12 | TM_CARD_TO_HOST |
            TM_MULTI_BLOCK | RSP_48 | CMD_ISDATA,
            lba);
}

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

    dmb();

    // reset host controller
    *EMMC_CONTROL0 = 0;
    *EMMC_CONTROL1 = CTL1_SRST_HC;
    *EMMC_CONTROL2 = 0;
    while (*EMMC_CONTROL1 & CTL1_SRST_HC);

    // configure clock and wait for it to become ready
    *EMMC_CONTROL1 = CTL1_CLK_INTLEN | CTL1_CLK_GENSEL | (1<<15) | CTL1_DATA_TOUNIT_DISABLED;
    while ((*EMMC_CONTROL1 & CTL1_CLK_STABLE) == 0);

    // enable clock
    *EMMC_CONTROL1 |= CTL1_CLK_EN;

    // enable interrupts
    *EMMC_INT_MASK = INT_ALL;

    // CMD0
    if (sendcmd(CMD_GO_IDLE_STATE, RSP_NONE, 0)) {
        panic("sdinit - cmd0");
    }

    // CMD8
    // TODO: support cards that don't respond to CMD8
    if (sendcmd(CMD_SEND_IF_COND, RSP_48_BUSY, VHS_27_36 | CHECK_PATTERN)) {
        panic("sdinit - cmd8");
    }

    // CMD8 echos back its arguments on success
    if (*EMMC_RESP0 != (VHS_27_36 | CHECK_PATTERN)) {
        panic("sdinit - cmd8 response");
    }

    int ready = 0;
    while (!ready) {
        // Maybe set card to host transfer mode? It's present in pios, but I
        // don't think its necessary.
        sendcmd(CMD_APP, RSP_48, 0);

        sendcmd(ACMD_SD_SEND_OP_COND, RSP_48, OCR_HCS | OCR_ALL_VOLTAGES);

        // OCR_BUSY is LOW when when busy, not high.
        ready = *EMMC_RESP0 & OCR_BUSY;
    }

    card.sdhc = (*EMMC_RESP0 & OCR_CSS) == OCR_CSS;

    // CMD2
    if (sendcmd(CMD_ALL_SEND_CID, RSP_136, 0)) {
        panic("sdinit - cmd2");
    }

    // CMD3
    if (sendcmd(CMD_SEND_RELATIVE_ADDR, RSP_48, 0)) {
        panic("sdinit - cmd3");
    }

    card.rca = *EMMC_RESP0 & 0xffff0000;

    // CMD7
    if (sendcmd(CMD_SELECT_CARD, RSP_48, card.rca)) {
        panic("sdinit - cmd7");
    }

    cprintf("emmc0 capabilities1=0x%x, capabilities2=0x%x\n", *EMMC_CAPABILITIES1, *EMMC_CAPABILITIES2);

    cprintf("sd0 initialized sdhc=%d, rca=0x%x\n", card.sdhc, card.rca >> 16);
}
