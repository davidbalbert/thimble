#define EXTMEM 0x100000                 // Start of extended memory -- kernel loaded here

#define KERNBASE 0xFFFF800000000000     // First kernel virtual address
#define KERNLINK (KERNBASE+EXTMEM)      // Kernel linked here

#define USERTOP 0x00007FFFFFFFFFFF	// Last user virtual address


#define PGSIZE 0x1000

#define NPDENTRIES 512

#define PTE_P 0x1
#define PTE_W 0x2
#define PTE_PS 0x80

#define V2P(x) ((x) - KERNBASE)
#define P2V(x) ((x) + KERNBASE)
