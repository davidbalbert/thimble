#define EXTMEM 0x100000                 // Start of extended memory -- kernel loaded here

#define KERNBASE 0xFFFF800000000000     // First kernel virtual address
#define KERNLINK (KERNBASE+EXTMEM)      // Kernel linked here

#define USERTOP 0x0000800000000000      // Top of user address space.
#define PHYSTOP 0x20000000              // Top of physical memory (512 MiB). TODO: autodetect this.
#define DEVSPACE 0xFE000000		// Just a guess for TOLUD, taken from xv6. Detect this eventually.
#define DEVTOP 0x100000000		// Devspace ends at 4 GB


#define PGSIZE 0x1000
#define KSTACKSIZE PGSIZE
#define USTACKSIZE PGSIZE

#define PTE_P  0x1      // present
#define PTE_W  0x2      // writable
#define PTE_U  0x4      // user accessible
#define PTE_PS 0x80     // large page size

// Bit offsets of paging structures in linear address
#define PML4SHIFT 39
#define PDPSHIFT  30
#define PDSHIFT   21
#define PTSHIFT   12

#define V2P(x) (((uintptr)(x)) - KERNBASE)
#define P2V(x) (((void *)(x)) + KERNBASE)

#define KERN_DPL     0
#define USER_DPL     3

#define SEG_KCODE    1
#define SEG_KDATA    2
#define SEG_UCODE32  3   // unused, but required by sysret
#define SEG_UDATA    4
#define SEG_UCODE    5
#define SEG_TSS      6

#define KB 1024l
#define MB (1024l*1024)
#define GB (1024l*1024*1024)

#ifndef __ASSEMBLER__

extern char end[];  // End of kernel, provided by linker
extern char data[]; // Start of kernel data section, provided by linker

#endif
