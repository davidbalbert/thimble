// ahci.c
int  ahcidetect(void);
void ahciread(uchar *addr, uint lba, uchar count);

// bootide.c
void ideread(uchar *addr, uint lba, uchar count);

// console.h
void cprintf(char *fmt, ...);
void cclear(void);
void cputs(char *s);
void cputc(uchar c);

// pci.c
typedef struct PciFunction PciFunction;
typedef int (*pciiter)(PciFunction *f);

char *pciclass(uchar code);
void pcieach(pciiter f);
uint pcibar(PciFunction *f, uchar bar);
