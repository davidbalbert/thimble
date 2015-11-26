#include "types.h"

#include "bootdefs.h"
#include "pci.h"
#include "x86.h"

static struct {
    PciFunction pci;
} hba; // Host bus adapter

static int ahcifound = 0;

static int
findahci(PciFunction *f)
{
    if (f->class == PCI_C_STORAGE && f->subclass == PCI_SC_AHCI) {
        ahcifound = 1;
        hba.pci = *f;

        return 0;
    } else {
        return 1;
    }
}

void
ahciread(uchar *addr, uint lba, uchar count)
{
    cprintf("AHCI not implemented yet");
    for (;;)
        hlt();
}

int
ahcidetect(void)
{
    uint *hbabase;

    pcieach(findahci);

    if (ahcifound) {
        hbabase = (uint *)(ulong)pcibar(&hba.pci, 5);
        cprintf("bar5: %p\n", hbabase);
        cprintf("*bar5: %p\n", *hbabase);
    }

    return ahcifound;
}
