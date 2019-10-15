static inline void
wfi(void)
{
    asm volatile("wfi");
}

static inline void
wfe(void)
{
    asm volatile("wfe");
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

// load-acquire exclusive
static inline uint
ldaxr(uint *p)
{
    uint res;

    asm volatile("ldaxr %[res], [%[p]]" : [res] "=r" (res) : [p] "r" (p));

    return res;
}

// store exclusive
static inline u32
stxr(uint *p, uint v)
{
    u32 status;

    asm volatile("stxr %w[status], %[v], [%[p]]" : [status] "=&r" (status) : [v] "r" (v), [p] "r" (p) : "memory");

    return status;
}

// store release
static inline void
stlr(uint *p, uint v)
{
    asm volatile("stlr %[v], [%[p]]" : : [v] "r" (v), [p] "r" (p) : "memory");
}
