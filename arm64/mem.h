#define KERNPHYS 0x80000 								// Kernel physical address

#define KERNBASE 0xFFFF000000000000     // first kernel virtual address
#define KERNLINK (KERNBASE+KERNPHYS)		// Kernel linked here

#define USERTOP  0x0001000000000000     // Top of user address space

#define PGSIZE 0x1000

#define V2P(x) (((uintptr)(x)) - KERNBASE)
#define P2V(x) (((void *)(x)) + KERNBASE)

#define TCL_EL1_T0SZ_48BIT 16           // 48 bit user space virtual addresses
#define TCL_EL1_T1SZ_48BIT (16 << 16)   // 48 bit kernel space virtual addresses
#define TCL_EL1_TG0_4K (0 << 14)        // 4 KB pages in user space
#define TCL_EL1_TG1_4K (0 << 30)        // 4 KB pages in kernel space

#define PTE_TABLE 0b11
#define PTE_BLOCK 0b01
#define PTE_L3_BLOCK 0b11
