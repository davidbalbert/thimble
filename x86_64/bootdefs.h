// ahci.c
int  ahcidetect(void);
void ahciread(byte *addr, u64 lba, u16 count);

// bootide.c
void ideread(byte *addr, u32 lba, byte count);

// console.c
typedef struct Console Console;

Console *uart_console;
Console *vga_console;

void cinit(Console *c);
void cprintf(char *fmt, ...);
void cclear(void);
void cputs(char *s);
void cputc(byte c);
void cvprintf(char *fmt, va_list ap);

// klibc.c
void *memset(void *p, int c, usize len);
void *memzero(void *p, usize len);
int isdigit(int c);
long strtol(char *s, char **endptr, int base);

// pci.c
typedef struct PciFunction PciFunction;
typedef int (*pciiter)(PciFunction *f);

char *pciclass(byte code);
void pcieach(pciiter f);
u32 pcibar(PciFunction *f, byte bar);

// stage2.c
void panic(char *fmt, ...) __attribute__((noreturn));
