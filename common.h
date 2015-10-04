// console.c
void panic(char *s);
void cprintf(char *fmt, ...);
void cclear(void);
void cputs(char *s);

// kbd.c
void kbdinit(void);
void handlekbd(void);

// pic.c
void picinit(void);
void picenable(uchar irq);

// trap.c
void trapinit(void);
