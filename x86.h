static inline void
hlt(void)
{
    asm volatile("hlt");
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

static inline void
ljmpstart()
{
    asm volatile("mv $0x100000, %eax; jmp %eax");
}
