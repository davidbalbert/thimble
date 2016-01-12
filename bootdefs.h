// ahci.c
int  ahcidetect(void);
void ahciread(uchar *addr, ulong lba, ushort count);

// bootide.c
void ideread(uchar *addr, uint lba, uchar count);

// console.c
void cprintf(char *fmt, ...);
void cclear(void);
void cputs(char *s);
void cputc(uchar c);

// klibc.c
void *memset(void *p, int c, size_t len);
void *memzero(void *p, size_t len);

// pci.c
typedef struct PciFunction PciFunction;
typedef int (*pciiter)(PciFunction *f);

char *pciclass(uchar code);
void pcieach(pciiter f);
uint pcibar(PciFunction *f, uchar bar);

// stage2.c
void panic(char *s) __attribute__((noreturn));
