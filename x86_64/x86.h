static inline void
hlt(void)
{
    asm volatile("hlt");
}

static inline void
outb(u16 port, byte data)
{
    asm volatile("outb %0, %1" : : "a" (data), "d" (port));
}

static inline void
outl(u16 port, u32 data)
{
    asm volatile("outl %0, %1" : : "a" (data), "d" (port));
}

static inline byte
inb(u16 port)
{
    byte data;

    asm volatile("inb %1, %0" : "=a" (data) : "d" (port));
    return data;
}

static inline u32
inl(u16 port)
{
    u32 data;

    asm volatile("inl %1, %0" : "=a" (data) : "d" (port));
    return data;
}

static inline void
insw(u16 port, void *addr, u32 count)
{
    asm volatile("cld; rep insw" :
                 "=D" (addr), "=c" (count) :
                 "d" (port), "0" (addr), "1" (count) :
                 "cc", "memory");
}

static inline void
stosb(void *dst, byte c, u64 len)
{
    asm volatile("cld; rep stosb" :
                 "=D" (dst), "=c" (len) :
                 "0" (dst), "1" (len), "a" (c) :
                 "cc", "memory");
}

static inline u64
readrflags(void)
{
    u64 rflags;
    asm volatile("pushfq; pop %0" : "=r" (rflags));
    return rflags;
}

static inline u64
readcr2(void)
{
    u64 cr2;
    asm volatile("mov %%cr2, %0" : "=a" (cr2));
    return cr2;
}


static inline u32
xchg(u32 *addr, u32 val)
{
    u32 ret;

    // xv6 has "cc" in the clobbers list, but the Intel
    // programmers manual says that xchg doesn't modify any flags.
    // I think it's fine to leave "cc" out, but I'm putting this
    // note here just in case.
    asm volatile("xchg %0, %1" :
                 "+m" (*addr), "=r" (ret) :
                 "1" (val));

    return ret;
}

typedef struct __attribute__((packed)) {
    u16 limit;
    u64 base;
} TableDescription;

struct InterruptGate;
typedef struct InterruptGate InterruptGate;

static inline void
lidt(InterruptGate *g, u32 size)
{
    TableDescription idtr;
    idtr.limit = size - 1;
    idtr.base = (u64)g;

    asm volatile("lidt (%0)" : : "r" (&idtr));
}

struct SegmentDescriptor;
typedef struct SegmentDescriptor SegmentDescriptor;

static inline void
lgdt(SegmentDescriptor *d, u32 size)
{
    TableDescription gdtr;
    gdtr.limit = size - 1;
    gdtr.base = (u64)d;

    asm volatile("lgdt (%0)" : : "r" (&gdtr));
}

static inline void
ltr(u16 sel)
{
    asm volatile("ltr %0" : : "r" (sel));
}

static inline void
lcr3(uintptr val)
{
    asm volatile("mov %0, %%cr3" : : "r" (val));
}
