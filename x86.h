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

static inline ushort
inw(ushort port)
{
    ushort data;

    asm volatile("in %1, %0" : "=a" (data) : "d" (port));
    return data;
}
