// x86_64/syscallasm.S
void sysinit(void);

// x86_64/vm.c
void *uva2ka(Pte *pgmap, void *addr);

// x86_64/kbd.c
void kbdinit(void);
void handlekbd(void);
