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

char *pciclass(uchar code);
void pcieach(int (*f)(PciFunction *));
