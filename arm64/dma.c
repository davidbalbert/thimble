#include "u.h"

#include "archdefs.h"
#include "arm64.h"
#include "defs.h"
#include "dma.h"
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

void
dmainit(void)
{
    int i;

    dmb();

    for (i = 0; i < 15; i++) {
        *R(CS, i) = CS_RESET;
    }

    // done is a bitmask with 15 bits, one per chanel
    int done = 0;
    while (done != 0x7FFF) {
        for (i = 0; i < 15; i++) {
            if (done & (1 << i)) {
                continue;
            }

            if ((*R(CS, i) & CS_RESET) == 0) {
                done |= (1 << i);
            }
        }
    }
}

void
dmastart(int chan, int dev, int dir, void *src, void *dst, usize len)
{
    dmb();

    if (chan < 0 || chan > 14) {
        panic("dma_start - chan must be between 0 and 14");
    }

    ControlBlock cb __attribute__ ((__aligned__(32)));

    memzero(&cb, sizeof(ControlBlock));

    switch(dir) {
        case DMA_D2M:
            cb.ti = DMA_TI_DEST_INC | DMA_TI_SRC_DREQ;
            cb.src_addr = busaddr_p(v2p(src));
            cb.dst_addr = busaddr_mem(v2p(dst));
            break;
        case DMA_M2D:
            panic("dmastart - M2D unsupported");
            break;
        case DMA_M2M:
            panic("dmastart - M2M unsupported");
            break;
    }

    cb.ti |=  DMA_TI_PERMAP(dev) | DMA_TI_INTEN;
    cb.len = len;
    cb.next = 0;

    dsb();

    *R(CONBLK_AD, chan) = busaddr_mem(v2p(&cb));
    *R(CS, chan) = CS_ACTIVE;

    while ((*R(CS, chan) & (CS_END | CS_ERROR)) == 0)
        ;

    if (*R(CS, chan) & CS_ERROR) {
        panic("dma_start - dma error");
    }

    *R(CS, chan) = CS_END | CS_INT;
}
