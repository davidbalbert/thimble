static inline void
wfi(void)
{
    asm volatile("wfi");
}

static inline int
el(void)
{
    int res;
    asm volatile("mrs %[res], CurrentEL" : [res] "=r" (res));

    return (res >> 2) & 0x3;
}
