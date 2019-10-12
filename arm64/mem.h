#define KERNPHYS 0x80000 								// Kernel physical address

#define KERNBASE 0xFFFF000000000000     // first kernel virtual address
#define KERNLINK (KERNBASE+KERNPHYS)		// Kernel linked here

#define USERTOP  0x0001000000000000     // Top of user address space

#define PGSIZE 0x1000

#define KSTACKSIZE PGSIZE

#define V2P(x) (((uintptr)(x)) - KERNBASE)
#define P2V(x) (((void *)(x)) + KERNBASE)

#define PTE_TABLE 0x3
#define PTE_PAGE  0x3
#define PTE_BLOCK 0x1

#define PTE_DEVICE_nGnRnE (0 << 2) // ATTR0
#define PTE_DEVICE        (1 << 2) // ATTR1
#define PTE_NON_CACHEABLE (2 << 2) // ATTR2
#define PTE_CACHEABLE     (3 << 2) // ATTR3

#define PTE_U   (1 << 6)  // user access
#define PTE_RO  (1 << 7)  // read-only

#define PTE_ISH (3 << 8)  // inner sharable

#define PTE_AF  (1 << 10) // access flag. If it's not set, an access fault will be generated

#define PTE_PXN (1 << 53) // priveledged execute-never
#define PTE_UXN (1 << 54) // user execute-never

#define KB 1024l
#define MB (1024l*1024)
#define GB (1024l*1024*1024)
