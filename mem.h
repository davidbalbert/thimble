#define EXTMEM 0x100000                 // Start of extended memory -- kernel loaded here

#define KERNBASE 0xFFFF800000000000     // First kernel virtual address
#define KERNLINK (KERNBASE+EXTMEM)      // Kernel linked here

#define USERTOP 0x0000800000000000      // Top of user address space.
#define PHYSTOP 0x20000000              // Top of physical memory (512 MiB). TODO: autodetect this.


#define PGSIZE 0x1000

#define PTE_P 0x1
#define PTE_W 0x2
#define PTE_PS 0x80

#define V2P(x) ((x) - KERNBASE)
#define P2V(x) ((x) + KERNBASE)

#define SEG_KCODE 1
#define SEG_KDATA 2
#define SEG_UCODE 3
#define SEG_UDATA 4
#define SEG_TSS	  5

#ifndef __ASSEMBLER__

extern char end[]; // End of kernel, provided by linker

#endif
