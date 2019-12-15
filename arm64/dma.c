#include "u.h"

#include "archdefs.h"
#include "arm64.h"
#include "defs.h"
#include "dma.h"
#include "irq.h"
#include "mem.h"

#include "bcm2837.h"

#define DMA0_BASE (PBASE+0x7000)

#define R(r, chan)     ((volatile u32 *)(DMA0_BASE + chan*0x100 + r))
#define DMA_INT_STATUS ((volatile u32 *)(DMA0_BASE+0xFE0))
#define DMA_ENABLE     ((volatile u32 *)(DMA0_BASE+0xFF0))

#define CS          0x00
#define CONBLK_AD   0x4

#define CS_ACTIVE   (1 << 0)
#define CS_END      (1 << 1)
#define CS_INT      (1 << 2)
#define CS_ERROR    (1 << 8)
#define CS_RESET    (1 << 31)

struct ControlBlock {
    u32 ti; // transfer information
    u32 src_addr;
    u32 dst_addr;
    u32 len;
    u32 stride;
    u32 next;
    u32 res[2]; // reserved - set to zero
};
typedef struct ControlBlock ControlBlock;

struct Ctrl {
    ControlBlock *cb;
};
typedef struct Ctrl Ctrl;

Ctrl ctrls[DMA_NCHAN];

void
dmastart(int chan, int dev, int dir, void *src, void *dst, usize len)
{
    dmb();

    if (chan < 0 || chan >= DMA_NCHAN) {
        panic("dmastart - chan must be between 0 and %d", DMA_NCHAN-1);
    }

    Ctrl *ctrl = &ctrls[chan];

    if (ctrl->cb == nil) {
        // TODO: 32 byte alligned malloc
        ctrl->cb = kalloc();
        if (ctrl->cb == nil) {
            panic("dmastart - alloc cb");
        }
        memzero(ctrl->cb, sizeof(ControlBlock));

        *DMA_ENABLE |= (1 << chan);

        *R(CS, chan) = CS_RESET;
        while (*R(CS, chan) & CS_RESET)
            ;

        intenable(IRQ_DMA(chan));
    }

    switch(dir) {
        case DMA_D2M:
            ctrl->cb->ti = DMA_TI_DEST_INC | DMA_TI_SRC_DREQ;
            ctrl->cb->src_addr = busaddr_p(v2p(src));
            ctrl->cb->dst_addr = busaddr_mem(v2p(dst));
            break;
        case DMA_M2D:
            panic("dmastart - M2D unsupported");
            break;
        case DMA_M2M:
            panic("dmastart - M2M unsupported");
            break;
    }

    ctrl->cb->ti |=  DMA_TI_PERMAP(dev) | DMA_TI_INTEN;
    ctrl->cb->len = len;
    ctrl->cb->next = 0;

    dsb();

    *R(CONBLK_AD, chan) = busaddr_mem(v2p(ctrl->cb));

    *DMA_INT_STATUS = 0;
    *R(CS, chan) = CS_INT;

    dmb();

    *R(CS, chan) = CS_ACTIVE;
}

void
dmawait(int chan, SpinLock *l)
{
    if (!holding(l)) {
        panic("dmawait - not holding lock");
    }

    dmb();

    if (chan < 0 || chan >= DMA_NCHAN) {
        panic("dmastart - chan must be between 0 and %d", DMA_NCHAN-1);
    }

    Ctrl *ctrl = &ctrls[chan];

    sleep(ctrl, l);

    if (*R(CS, chan) & CS_ERROR) {
        panic("dmastart - dma error");
    }

    *R(CS, chan) = CS_END | CS_INT;
}

void
dmaintr(int chan)
{
    if (chan < 0 || chan >= DMA_NCHAN) {
        panic("dmaintr - chan must be between 0 and %d", DMA_NCHAN-1);
    }

    Ctrl *ctrl = &ctrls[chan];

    *R(CS, chan) = CS_INT;

    wakeup(ctrl);
}
