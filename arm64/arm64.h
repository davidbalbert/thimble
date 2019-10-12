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

static inline void
st_vbar_el1(void *p)
{
    asm volatile("msr vbar_el1, %[p]" : : [p] "r" (p));
}
