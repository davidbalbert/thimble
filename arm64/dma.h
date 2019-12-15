// Only use the full channels (0-6), not the "lite" ones.
#define DMA_NCHAN 7

#define DMA_M2M 0
#define DMA_D2M 1
#define DMA_M2D 2

#define DMA_CHAN_EMMC 0
#define DMA_DEV_EMMC 11

#define DMA_TI_INTEN     (1 << 0) // interrupt enable
#define DMA_TI_DEST_INC  (1 << 4)
#define DMA_TI_DEST_DREQ (1 << 6)
#define DMA_TI_SRC_INC   (1 << 8)
#define DMA_TI_SRC_DREQ  (1 << 10)
#define DMA_TI_PERMAP(n) ((n&0x1F) << 16)
