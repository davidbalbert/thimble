#define KERNPHYS 0x80000 								// Kernel physical address

#define KERNBASE 0xFFFF000000000000     // first kernel virtual address
#define KERNLINK (KERNBASE+KERNPHYS)		// Kernel linked here

#define USERTOP  0x0001000000000000     // Top of user address space
#define PHYSTOP  0x3F000000             // Top of physical memory. Raspi 3 has 1 GiB, but it seems like only 1023 MiB are mapped in (https://www.raspberrypi.org/forums/viewtopic.php?t=186090). TODO: autodetect this.
#define DEVSPACE 0x3F000000             // SoC peripherals (Raspi3)
#define DEVTOP   0x40000000             // SoC Peripheral space ends at 1 GiB
#define LSPACE   0x40000000             // Local peripherals (Raspi3)
#define LTOP     0x40020000             // End of local peripherals

#define PGSIZE 0x1000

#define KSTACKSIZE PGSIZE

#define V2P(x) (((uintptr)(x)) - KERNBASE)
#define P2V(x) (((void *)(x)) + KERNBASE)

#define PTE_P     (1 << 0)
#define PTE_TABLE (1 << 1)
#define PTE_PAGE  (1 << 1)
#define PTE_BLOCK (0 << 1)


#define PTE_DEVICE_nGnRnE (0 << 2) // ATTR0
#define PTE_DEVICE_GRE    (1 << 2) // ATTR1
#define PTE_NON_CACHEABLE (2 << 2) // ATTR2
#define PTE_CACHEABLE     (3 << 2) // ATTR3

#define PTE_U   (1 << 6)  // user access
#define PTE_RO  (1 << 7)  // read-only
#define PTE_W   (0 << 7)  // read/write

#define PTE_ISH (3 << 8)  // inner sharable

#define PTE_AF  (1 << 10) // access flag. If it's not set, an access fault will be generated

#define PTE_PXN (1 << 53) // priveledged execute-never
#define PTE_UXN (1 << 54) // user execute-never

#define KB 1024l
#define MB (1024l*1024)
#define GB (1024l*1024*1024)

#ifndef __ASSEMBLER__

extern char end[];  // End of kernel, provided by linker
extern char data[]; // Start of kernel data section, provided by linker

#endif
