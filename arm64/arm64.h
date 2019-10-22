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

static inline u64
readdaif(void)
{
    u64 res;
    asm volatile("mrs %[res], DAIF" : [res] "=r" (res));

    return res;
}

static inline void
cli(void)
{
    asm volatile("msr DAIFSet, #15");
}

static inline void
sti(void)
{
    asm volatile("msr DAIFClr, #15");
}

static inline void
lttbr0(uintptr pgmap)
{
    asm volatile("msr ttbr0_el1, %[pgmap]" : : [pgmap] "r" (pgmap));
}

static inline void
lttbr1(uintptr pgmap)
{
    asm volatile("msr ttbr1_el1, %[pgmap]" : : [pgmap] "r" (pgmap));
}

static inline void
dsb(void)
{
    asm volatile("dsb ish");
}

static inline void
isb(void)
{
    asm volatile("isb");
}

static inline void
tlbi(void)
{
    asm volatile("tlbi vmalle1is");
}
