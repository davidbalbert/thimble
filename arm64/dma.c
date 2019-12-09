#include "u.h"

#include "arm64.h"
#include "dma.h"
#include "mem.h"

#include "bcm2837.h"

#define DMA0_BASE (PBASE+0x7000)

#define R(r, chan) ((volatile u32 *)(DMA0_BASE + chan*0x100 + r))

#define CS          0x00
#define CONBLK_AD   0x4

#define CS_RESET    (1 << 31)

void
dma_init(void)
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
dma_start(int chan, DmaControlBlock *cb)
{
    dmb();


}
