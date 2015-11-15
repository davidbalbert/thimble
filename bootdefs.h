// bootide.c
void readsects(uchar *addr, uint lba, uchar count);

// console.h
void cprintf(char *fmt, ...);
void cclear(void);
void cputs(char *s);
void cputc(uchar c);

// pci.c
typedef struct PciFunction PciFunction;

char *pciclass(uchar code);
void pcieach(int (*f)(PciFunction *));
