static inline void
hlt(void)
{
    asm volatile("hlt");
}

static inline void
cli(void)
{
    asm volatile("cli");
}

static inline void
sti(void)
{
    asm volatile("sti");
}

static inline void
outb(ushort port, uchar data)
{
    asm volatile("outb %0, %1" : : "a" (data), "d" (port));
}

static inline uchar
inb(ushort port)
{
    uchar data;

    asm volatile("in %1, %0" : "=a" (data) : "d" (port));
    return data;
}

static inline void
insw(ushort port, void *addr, uint count)
{
    asm volatile("cld; rep insw" :
                 "=D" (addr), "=c" (count) :
                 "d" (port), "0" (addr), "1" (count) :
                 "cc", "memory");
}

static inline void
stosb(void *dst, uchar c, ulong len)
{
    asm volatile("cld; rep stosb" :
                 "=D" (dst), "=c" (len) :
                 "0" (dst), "1" (len), "a" (c) :
                 "cc", "memory");
}

static inline ulong
readrflags(void)
{
    ulong rflags;
    asm volatile("pushfq; pop %0" : "=r" (rflags));
    return rflags;
}

static inline uint
xchg(uint *addr, uint val)
{
    uint ret;

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
    ushort limit;
    ulong base;
} TableDescription;

struct InterruptGate;
typedef struct InterruptGate InterruptGate;

static inline void
lidt(InterruptGate *g, uint size)
{
    TableDescription idtr;
    idtr.limit = size - 1;
    idtr.base = (ulong)g;

    asm volatile("lidt (%0)" : : "r" (&idtr));
}

struct SegmentDescriptor;
typedef struct SegmentDescriptor SegmentDescriptor;

static inline void
lgdt(SegmentDescriptor *d, uint size)
{
    TableDescription gdtr;
    gdtr.limit = size - 1;
    gdtr.base = (ulong)d;

    asm volatile("lgdt (%0)" : : "r" (&gdtr));
}

static inline void
ltr(ushort sel)
{
    asm volatile("ltr %0" : : "r" (sel));
}

#define INT(v) asm volatile("int %0" : : "N" (v) : "cc", "memory")
