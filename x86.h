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
} IdtDesc;

static inline void
lidt(IdtDesc *idtr)
{
    asm volatile("lidt (%0)" : : "r" (idtr));
}

#define INT(v) asm volatile("int %0" : : "N" (v) : "cc", "memory")
