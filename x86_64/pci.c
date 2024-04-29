// PCI driver. Used by stage2 bootloader.
#include "u.h"

#include "pci.h"
#include "x86.h"

#define ADDR 0xCF8
#define DATA 0xCFC

#define ENABLE (1 << 31)

#define VENDORID 0x0
#define DEVICEID 0x2
#define SUBCLASS 0xA
#define CLASS    0xB
#define BAR0     0x10

#define NBUS  256
#define NDEV  32
#define NFUNC 8

#define MULTIFUNC (1 << 7)

static char *classes[] = {
    "Prehistoric device",
    "Mass Storage Controller",
    "Network Controller",
    "Display Controller",
    "Multimedia Controller",
    "Memory Controller",
    "Bridge Device",
    "Simple Communication Controller",
    "Base System Peripheral",
    "Input Device",
    "Docking Station",
    "CPU",
    "Serial Bus Controller",
    "Wireless Controller",
    "Intelligent I/O Controller",
    "Satellite Communication Controller",
    "Encryption/Decryption Controller",
    "Data Acquisition and Signal Processing Controller",
    [0xFF] = "Device does not fit defined class"
};

char *
pciclass(byte code)
{
    return classes[code] ? classes[code] : "Reserved";
}

static u32
pcireadl(byte bus, byte dev, byte func, byte offset)
{
    u64 addr;

    addr = ENABLE | (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC);

    outl(ADDR, addr);
    return inl(DATA);
}

static u16
pcireadw(byte bus, byte dev, byte func, byte offset)
{
    u32 data;

    data = pcireadl(bus, dev, func, offset);

    return offset % 4 == 0 ? data : data >> 16;
}

static byte
pcireadb(byte bus, byte dev, byte func, byte offset)
{
    u32 data;
    int shift = 8 * (offset % 4);

    data = pcireadl(bus, dev, func, offset);

    return data >> shift;
}

static int
ismultifunc(u16 vendorid)
{
    return vendorid & MULTIFUNC;
}

static int
checkdevice(byte bus, byte dev, int (*f)(PciFunction *))
{
    u16 vendorid;
    PciFunction func;
    int i;

    vendorid = pcireadw(bus, dev, 0, VENDORID);

    if (vendorid == 0xFFFF)
        return 1;

    func.vendorid = vendorid;
    func.bus = bus;
    func.dev = dev;
    func.func = 0;
    func.class = pcireadb(bus, dev, 0, CLASS);
    func.subclass = pcireadb(bus, dev, 0, SUBCLASS);

    if (f(&func) == 0)
        return 0;

    if (ismultifunc(vendorid)) {
        for (i = 1; i < NFUNC; i++)  {
            vendorid = pcireadw(bus, dev, i, VENDORID);

            if (vendorid == 0xFFFF)
                continue;

            func.vendorid = vendorid;
            func.func = i;
            func.class = pcireadb(bus, dev, i, CLASS);
            func.subclass = pcireadb(bus, dev, i, SUBCLASS);

            if (f(&func) == 0)
                return 0;
        }
    }

    return 1;
}

// Enumerates all present PCI functions, calling f with each one.
// If f returns 0, stop enumerating. Otherwise, continue.
void
pcieach(int (*f)(PciFunction *))
{
    int bus, dev, ret;

    for (bus = 0; bus < NBUS; bus++) {
        for (dev = 0; dev < NDEV; dev++) {
            ret = checkdevice(bus, dev, f);

            if (ret == 0)
                return;
        }
    }
}

// Returns base address register bar for function f.
u32
pcibar(PciFunction *f, byte bar)
{
    return pcireadl(f->bus, f->dev, f->func, BAR0 + 4*bar);
}
