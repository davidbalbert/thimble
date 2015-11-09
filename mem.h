#define EXTMEM 0x100000                 // Start of extended memory -- kernel loaded here

#define KERNBASE 0xFFFF800000000000     // First kernel virtual address
#define KERNLINK (KERNBASE+EXTMEM)      // Kernel linked here

#define USERTOP 0x0000800000000000      // Top of user address space.
#define PHYSTOP 0x20000000              // Top of physical memory (512 MiB). TODO: autodetect this.


#define PGSIZE 0x1000
#define KSTACKSIZE PGSIZE

#define PTE_P 0x1       // present
#define PTE_W 0x2       // writable
#define PTE_U 0x4       // user accessible
#define PTE_PS 0x80     // page size (4 MB)

#define V2P(x) ((x) - KERNBASE)
#define P2V(x) ((x) + KERNBASE)

#define USER_DPL     3

#define SEG_KCODE    1
#define SEG_KDATA    2
#define SEG_UCODE32  3   // unused, but required by sysret
#define SEG_UDATA    4
#define SEG_UCODE    5
#define SEG_TSS      6

#ifndef __ASSEMBLER__

extern char end[]; // End of kernel, provided by linker

#endif
