// ahci driver - is part of the boot loader and uses physical addresses
//
// How to modify this to be shared with the kernel:
// - pull out all static data into bootahci.c
//      - when the bootloader calls functions in ahci.c, use p2v on any pointers passed in
// - use p2v inside abar
// - when setting pointer fields in either the HBA registers or any of the
//   system data structures, use v2p.

#include "types.h"

#include "bootdefs.h"
#include "mem.h"
#include "pci.h"
#include "x86.h"

struct Port {
    uint clb;       // command list base address, 1K-byte aligned
    uint clbu;      // command list base address upper 32 bits
    uint fb;        // FIS base address, 256-byte aligned
    uint fbu;       // FIS base address upper 32 bits
    uint is;        // interrupt status
    uint ie;        // interrupt enable
    uint cmd;       // command and status
    uint rsv0;      // Reserved
    uint tfd;       // task file data
    uint sig;       // signature
    uint ssts;      // SATA status (SCR0:SStatus)
    uint sctl;      // SATA control (SCR2:SControl)
    uint serr;      // SATA error (SCR1:SError)
    uint sact;      // SATA active (SCR3:SActive)
    uint ci;        // command issue
    uint sntf;      // SATA notification (SCR4:SNotification)
    uint fbs;       // FIS-based switch control
    uint rsv1[11];
    uint vendor[4]; // vendor specific
};

typedef volatile struct Port Port;

struct HbaMemory {
    uint cap;       // Host capability
    uint ghc;       // Global host control
    uint is;        // Interrupt status
    uint pi;        // Port implemented
    uint vs;        // Version
    uint ccc_ctl;   // Command completion coalescing control
    uint ccc_pts;   // Command completion coalescing ports
    uint em_loc;    // Enclosure management location
    uint em_ctl;    // Enclosure management control
    uint cap2;      // Host capabilities extended
    uint bohc;      // BIOS/OS handoff control and status

    uchar rsv[116];

    uchar vendor[96];   // Vendor specific registers

    Port ports[0];   // Between 1 and 32 ports
};

typedef volatile struct HbaMemory HbaMemory;

struct CommandHeader {
    uint cfl:5;     // Command FIS length
    uint a:1;       // ATAPI
    uint w:1;       // Write
    uint p:1;       // Prefetchable

    uint r:1;       // Reset
    uint b:1;       // BIST
    uint c:1;       // Clear busy upon R_OK
    uint rsv0:1;    // Reserved
    uint pmp:4;     // Port multiplier port

    ushort prdtl;           // Physical region descriptor table length in entries
    volatile uint prdbc;    // Physical region descriptor byte count transferred. Inititalize to 0.

    uint ctba;      // Command table descriptor base address, 128-byte aligned
    uint ctbau;     // Command table descriptor base address upper 32 bits

    uint rsv1[4];
};

typedef struct CommandHeader CommandHeader;


struct PhysicalRegionDescriptor
{
    uint dba;       // Data base address
    uint dbau;      // Data base address upper 32 bits
    uint rsv0;      // Reserved

    uint dbc:22;    // Byte count, 4M max
    uint rsv1:9;    // Reserved
    uint i:1;       // Interrupt on completion
};

typedef struct PhysicalRegionDescriptor PhysicalRegionDescriptor;


#define NPRD 8

struct CommandTable
{
    uchar cfis[64];    // Command FIS
    uchar acmd[16];    // ATAPI command, 12 or 16 bytes
    uchar rsv[48];

    PhysicalRegionDescriptor prdt[NPRD];   // can contain 0 to 65535 entries
};

typedef struct CommandTable CommandTable;


struct ReceivedFisStorage {
    uchar   dsfis[28];      // dma setup fis
    uchar   pad0[4];

    uchar   psfis[20];      // pio setup (device to host) fis
    uchar   pad1[12];

    uchar   rfis[20];       // device to host register fis
    uchar   pad2[4];

    uchar   sdbfis[8];      // set device bits fis

    uchar   ufis[64];       // unknown fis

    // the spec's diagram has 95 bytes, but osdev has
    // 96 and I'd rather waste a byte than have a random
    // byte somewhere else overwritten.
    uchar   reserved[96];
};

typedef struct ReceivedFisStorage ReceivedFisStorage;



static struct {
    PciFunction pci;
    HbaMemory *base;
    int nports;         // number of ports
    int nslots;         // number of command slots per port
    int dma64;          // bool - supports 64-bit physical addresses for DMA
} hba; // Host bus adapter

#define NCMD 32

static CommandHeader cmdlist[NCMD] __attribute__((aligned(1*KB)));
static CommandTable cmdtbl __attribute__((algined(128)));
static ReceivedFisStorage fisstorage __attribute__((aligned(256)));

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

static int
findslot(Port *port)
{
    // ...
}

static CommandHeader *
getcmdlist(Port *port)
{
    return (CommandHeader*)((port->clbu << 32) + port->clb);
}

void
ahciread(uchar *addr, uint lba, uchar sectcount)
{
    int slot;
    Port *port;
    CommandHeader cmdhdr;

    port = &hba.base->ports[0];
    port->is = 0xFFFFFFFF;      // clear interrupt flags

    slot = findslot(port);
    cmdhdr = getcmdlist(port);



    // Set up the dma read command in the command list???
    // ...
    // port->cmd |= PORT_CMD_ST;
}


static HbaMemory *
abar(PciFunction *f)
{
    return (HbaMemory *)(ulong)(pcibar(f, 5) & 0xFFFFFFF0);
}

static char *
version(uint vs) {
    switch (vs) {
        case 0x0905:
            return "0.95";
        case 0x10000:
            return "1.0";
        case 0x10100:
            return "1.1";
        case 0x10200:
            return "1.2";
        case 0x10300:
            return "1.3";
        case 0x10301:
            return "1.3.1";
        default:
            return "Unknown version";
    }
}

static char *
speed(uint cap)
{
    // speed is bytes 23:20
    switch ((cap >> 20) & 0xF) {
        case 1:
            return "1.5 Gbps";
        case 2:
            return "3 Gbps";
        case 3:
            return "6 Gbps";
        default:
            return "Unknown speed";
    }
}

static void
yn(char *feature, int present)
{
    char *res = present ? "yes" : "no";

    cprintf("ahci: %s? %s\n", feature, res);
}

static int
popcount(ulong x)
{
    int count;
    for (count = 0; x; count++) {
        x &= x - 1; // clear least significant non-zero bit
    }
    return count;
}

#define CAP_S64A (1 << 31) // 64-bit addressing
#define CAP_SNCQ (1 << 30) // native command queuing
#define CAP_SAM  (1 << 18) // ahci only
#define CAP_SPM  (1 << 17) // port multiplier support

#define PORT_CMD_ST  (1 << 0)   // start processing the command list
#define PORT_CMD_FRE (1 << 4)   // fis receive enable
#define PORT_CMD_FR  (1 << 14)  // fis receive running
#define PORT_CMD_CR  (1 << 15)  // command list running

static void
checkalign(void *a, int alignment, char *msg)
{
    uintptr aa = (uintptr)a;

    if (aa & (alignment-1))
        panic(msg);
}


static void
portstart(Port *port)
{
    while (port->cmd & PORT_CMD_CR)
        ;

    port->cmd |= PORT_CMD_FRE;
    port->cmd |= PORT_CMD_ST;
}

static void
portstop(Port *port)
{
    port->cmd &= ~PORT_CMD_ST;

    while (port->cmd & (PORT_CMD_FR | PORT_CMD_CR))
        ;

    port->cmd &= ~PORT_CMD_FRE
}

static void
portinit(Port *port, CommandHeader *cl, ReceivedFisStorage *fisbase)
{
    uintptr cll, fisbasel;
    int i;

    checkalign(cl, 1*KB, "portinit - cl align");
    checkalign(fisbase, 256, "portinit - fisbase align");

    cll = (uintptr)cl;
    fisbasel = (uintptr)fisbase;

    if (!hba.dma64 && cll >= 4*GB)
        panic("portinit - cl");
    if (!hba.dma64 && fisbasel >= 4*GB)
        panic("portinit - fisbase");

    portstop(port);

    port->clb = (uint)cll;
    if (hba.dma64)
        port->clbu = (uint)(cll >> 32);

    port->fb = (uint)fisbasel;
    if (hba.dma64)
        port->fbu = (uint)(fisbasel >> 32);

    for (i = 0; i < NCMD; i++) {
        cl[i].prdtl = NPRD;
    }

    portstart(port);
}

int
ahcidetect(void)
{
    HbaMemory *base;
    Port *port;

    pcieach(findahci);

    if (ahcifound) {
        base = hba.base = abar(&hba.pci);
        hba.nports = popcount(base->pi);
        hba.nslots = ((base->cap >> 8) & 0x1F) + 1;
        hba.dma64  = base->cap & CAP_S64A;

        cprintf("ahci: found controller at pci%d.%d.%d: ", hba.pci.bus, hba.pci.dev, hba.pci.func);
        cprintf("version %s, %s, %d ports\n", version(base->vs), speed(base->cap), hba.nports);
        cprintf("ahci: %d command slots\n", hba.nslots);

        yn("64-bit addressing", base->cap & CAP_S64A);
        yn("native command queuing", base->cap & CAP_SNCQ);
        yn("ahci only", base->cap & CAP_SAM);
        yn("port multiplier support", base->cap & CAP_SPM);


        port = &base->ports[0];
        cprintf("ports[0].clb = %x\n", port->clb);
        cprintf("ports[0].clbu = %x\n", port->clbu);
        cprintf("ports[0].fb = %x\n", port->fb);
        cprintf("ports[0].fbu = %x\n", port->fbu);

        yn("ports[0] FIS Receive Enable", port->cmd & PORT_CMD_FRE);

        portinit(&base->ports[0], cmdlist, &fisstorage);
    }

    return ahcifound;
}
