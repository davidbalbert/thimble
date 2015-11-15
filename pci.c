// PCI driver. Used by stage2 bootloader.
#include "types.h"

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

static char *
classname(uchar code)
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

static void
checkdevice(uchar bus, uchar dev)
{
    ushort vendorid;
    uchar class;
    int func;

    vendorid = pcireadw(bus, dev, 0, PCI_VENDORID);

    if (vendorid == 0xFFFF)
        return;

    class = pcireadb(bus, dev, 0, PCI_CLASS);

    cprintf("pci%d.%d.0: %s\n", bus, dev, classname(class));

    if (ismultifunc(vendorid)) {
        for (func = 1; func < PCI_NFUNC; func++)  {
            vendorid = pcireadw(bus, dev, func, PCI_VENDORID);

            if (vendorid == 0xFFFF)
                continue;

            class = pcireadb(bus, dev, func, PCI_CLASS);

            cprintf("pci%d.%d.%d: %s\n", bus, dev, func, classname(class));
        }
    }
}

void
pcienumerate(void)
{
    int bus, dev;

    for (bus = 0; bus < PCI_NBUS; bus++) {
        for (dev = 0; dev < PCI_NDEV; dev++) {
            checkdevice(bus, dev);
        }
    }
}
