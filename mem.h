#define EXTMEM 0x100000                 // Start of extended memory -- kernel loaded here

#define KERNBASE 0xFFFF800000000000     // First kernel virtual address
#define KERNLINK (KERNBASE+EXTMEM)      // Kernel linked here

#define USERTOP 0x00007FFFFFFFFFFF      // Last user virtual address


#define PGSIZE 0x1000

#define NPDENTRIES 512

#define PTE_P 0x1
#define PTE_W 0x2
#define PTE_PS 0x80

#define V2P(x) ((x) - KERNBASE)
#define P2V(x) ((x) + KERNBASE)

#ifndef __ASSEMBLER__

typedef struct {
    uint offlow:16;     // bottom 16 bits of segment offset
    uint cs:16;         // code segment selector
    uint ist:3;         // interrupt stack table
    uint reserved1:5;   // all 0s.
    uint type:4;
    uint reserved2:1;   // 0
    uint dpl:2;         // descriptor privelege level
    uint p:1;           // present
    uint offmid:16;     // bits 16 - 31 of segment offset
    uint offhigh:32;    // bits 32 - 63 of segment offset
    uint reserved3:32;  // all 0s.
} InterruptGate;

typedef struct __attribute__((packed)) {
    ushort limit;
    ulong base;
} IdtDesc;

#endif
