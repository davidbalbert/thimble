// PCI driver. Used by stage2 bootloader.
#include "types.h"

#include "pci.h"
#include "x86.h"

#define PCI_ADDR 0xCF8
#define PCI_DATA 0xCFC

#define PCI_ENABLE (1 << 31)

#define PCI_VENDORID 0x0
#define PCI_DEVICEID 0x2
#define PCI_SUBCLASS 0xA
#define PCI_CLASS    0xB

#define PCI_NBUS  256
#define PCI_NDEV  32
#define PCI_NFUNC 8

#define PCI_MULTIFUNC (1 << 7)

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
    [0xFF] "Device does not fit defined class"
};

char *
pciclass(uchar code)
{
    return classes[code] ? classes[code] : "Reserved";
}

uint
pcireadl(uchar bus, uchar dev, uchar func, uchar offset)
{
    ulong addr;

    addr = PCI_ENABLE | (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC);

    outl(PCI_ADDR, addr);
    return inl(PCI_DATA);
}

ushort
pcireadw(uchar bus, uchar dev, uchar func, uchar offset)
{
    uint data;

    data = pcireadl(bus, dev, func, offset);

    return offset % 4 == 0 ? data : data >> 16;
}

uchar
pcireadb(uchar bus, uchar dev, uchar func, uchar offset)
{
    uint data;
    int shift = 8 * (offset % 4);

    data = pcireadl(bus, dev, func, offset);

    return data >> shift;
}

static int
ismultifunc(ushort vendorid)
{
    return vendorid & PCI_MULTIFUNC;
}

void cprintf(char *, ...);

static int
checkdevice(uchar bus, uchar dev, int (*f)(PciFunction *))
{
    ushort vendorid;
    PciFunction func;
    int i;

    vendorid = pcireadw(bus, dev, 0, PCI_VENDORID);

    if (vendorid == 0xFFFF)
        return 1;

    func.vendorid = vendorid;
    func.bus = bus;
    func.dev = dev;
    func.func = 0;
    func.class = pcireadb(bus, dev, 0, PCI_CLASS);
    func.subclass = pcireadb(bus, dev, 0, PCI_SUBCLASS);

    if (f(&func) == 0)
        return 0;

    if (ismultifunc(vendorid)) {
        for (i = 1; i < PCI_NFUNC; i++)  {
            vendorid = pcireadw(bus, dev, i, PCI_VENDORID);

            if (vendorid == 0xFFFF)
                continue;

            func.vendorid = vendorid;
            func.func = i;
            func.class = pcireadb(bus, dev, i, PCI_CLASS);
            func.subclass = pcireadb(bus, dev, i, PCI_SUBCLASS);

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

    for (bus = 0; bus < PCI_NBUS; bus++) {
        for (dev = 0; dev < PCI_NDEV; dev++) {
            ret = checkdevice(bus, dev, f);

            if (ret == 0)
                return;
        }
    }
}
