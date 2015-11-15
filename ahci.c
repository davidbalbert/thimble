#include "types.h"

#include "bootdefs.h"
#include "pci.h"
#include "x86.h"

static struct {
    uchar bus;
    uchar dev;
    uchar func;
} hba; // Host bus adapter

static int ahcifound = 0;

static int
findahci(PciFunction *f)
{
    if (f->class == PCI_C_STORAGE && f->subclass == PCI_SC_AHCI) {
        ahcifound = 1;

        hba.bus = f->bus;
        hba.dev = f->dev;
        hba.func = f->func;

        return 0;
    } else {
        return 1;
    }
}

void
ahciread(uchar *addr, uint lba, uchar count)
{
    cclear();
    cprintf("AHCI not implemented yet");
    for (;;)
        hlt();
}

int
ahcidetect(void)
{
    pcieach(findahci);

    return ahcifound;
}
