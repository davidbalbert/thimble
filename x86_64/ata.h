#define ATA_SECTSIZE 512

#define ATA_LBA_MODE (1<<6)

#define ATA_MAXLBA28 ((1 << 28) - 1)
#define ATA_MAXLBA48 ((1l << 48) - 1)

// status
#define ATA_STS_ERR (1 << 0)    // error
#define ATA_STS_DRQ (1 << 3)    // data transfer requested
#define ATA_STS_BSY (1 << 7)    // busy

// commands
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_READ_DMA_EXT 0x25
