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
    asm volatile("mrs %x[res], CurrentEL" : [res] "=r" (res));

    return (res >> 2) & 0x3;
}

static inline void
st_vbar_el1(void *p)
{
    asm volatile("msr vbar_el1, %[p]" : : [p] "r" (p));
}

// load-acquire exclusive
static inline u32
ldaxr(u32 *p)
{
    u32 res;

    asm volatile("ldaxr %w[res], [%[p]]" : [res] "=r" (res) : [p] "r" (p));

    return res;
}

// store exclusive
static inline u32
stxr(u32 *p, u32 v)
{
    u32 status;

    asm volatile("stxr %w[status], %w[v], [%[p]]" : [status] "=&r" (status) : [v] "r" (v), [p] "r" (p) : "memory");

    return status;
}

// store release
static inline void
stlr(u32 *p, u32 v)
{
    asm volatile("stlr %w[v], [%[p]]" : : [v] "r" (v), [p] "r" (p) : "memory");
}

static inline u64
readdaif(void)
{
    u64 res;
    asm volatile("mrs %[res], DAIF" : [res] "=r" (res));

    return res;
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
dmb(void)
{
    asm volatile("dmb ish");
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

// count leading zeroes
static inline byte
clz(u64 n)
{
    u64 res;
    asm volatile("clz %[res], %[n]" : [res] "=r" (res) : [n] "r" (n));

    return res;
}

// count trailing zeroes
static inline byte
ctz(u64 n)
{
    return 64 - clz(n) - 1;
}

// read the frequency of the system counter
static inline u64
cntfrq(void)
{
    u64 res;
    asm volatile("mrs %0, cntfrq_el0" : "=r" (res));

    return res;
}

// current system count value
static inline u64
cntct(void)
{
    u64 res;
    asm volatile("mrs %0, cntpct_el0" : "=r" (res));

    return res;
}

// value of CNTP_CTL_EL0, the physical timer control register.
static inline u64
cntpctl(void)
{
    u64 res;
    asm volatile("mrs %0, cntp_ctl_el0" : "=r" (res));

    return res;
}

// set CNTP_CT_EL0, the physical timer control register
static inline void
st_cntpctl(u64 val)
{
    asm volatile("msr cntp_ctl_el0, %0" :: "r" (val));
}

static inline void
st_cntptval(u64 val)
{
    asm volatile("msr cntp_tval_el0, %0" :: "r" (val));
}

static inline u64
spsel(void)
{
    u64 res;
    asm volatile("mrs %0, spsel" : "=r" (res));

    return res;
}

static inline u64
sp(void)
{
    u64 res;
    asm volatile("mov %0, sp" : "=r" (res));

    return res;
}

struct TrapFrame {
    u64 sp_el1;
    u64 sp_el0;

    u64 spsr;
    u64 far;
    u64 esr;
    u64 elr;

    u64 x0;
    u64 x1;
    u64 x2;
    u64 x3;
    u64 x4;
    u64 x5;
    u64 x6;
    u64 x7;
    u64 x8;
    u64 x9;
    u64 x10;
    u64 x11;
    u64 x12;
    u64 x13;
    u64 x14;
    u64 x15;
    u64 x16;
    u64 x17;
    u64 x18;
    u64 x19;
    u64 x20;
    u64 x21;
    u64 x22;
    u64 x23;
    u64 x24;
    u64 x25;
    u64 x26;
    u64 x27;
    u64 x28;
    u64 x29;
    u64 x30;

    u64 type;
};
typedef struct TrapFrame TrapFrame;
