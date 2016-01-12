// ahci driver - is part of the boot loader and uses physical addresses
//
// How to modify this to be shared with the kernel:
// - pull out all static data into bootahci.c
//      - when the bootloader calls functions in ahci.c, use p2v on any pointers passed in
// - use p2v inside abar
// - when setting pointer fields in either the HBA registers or any of the
//   system data structures, use v2p.

#include "types.h"

#include "ata.h"
#include "bootdefs.h"
#include "mem.h"
#include "pci.h"
#include "x86.h"

// capabilities
#define CAP_S64A (1 << 31) // 64-bit addressing
#define CAP_SNCQ (1 << 30) // native command queuing
#define CAP_SAM  (1 << 18) // ahci only
#define CAP_SPM  (1 << 17) // port multiplier support

// port command
#define PORT_CMD_ST  (1 << 0)   // start processing the command list
#define PORT_CMD_FRE (1 << 4)   // fis receive enable
#define PORT_CMD_FR  (1 << 14)  // fis receive running
#define PORT_CMD_CR  (1 << 15)  // command list running

// interrupt status
#define PORT_IS_TFES (1 << 30) // interrupt due to task file error bit set

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


#define PRDSIZE (4*MB) // max byte count in a PRDT entry

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

    // WARING! ahciread relies on sizeof(CommandTable). If we ever
    // dynamically allocate Command Tables, we must change ahciread!
    PhysicalRegionDescriptor prdt[NPRD];   // can contain 0 to 65535 entries
};

typedef struct CommandTable CommandTable;


#define FIS_TYPE_REG_H2D 0x27

struct FisRegisterH2D {
    uchar type;
    uint  rsv0:7;
    uint  c:1;      // 1 - command register update, 0 = device controle update
    uchar command;  // command register
    uchar feat;     // feature register 7:0
    uchar lba0;     // lba 7:0
    uchar lba1;     // lba 15:8
    uchar lba2;     // lba 23:16
    uchar device;   // device register
    uchar lba3;     // lba 31:24
    uchar lba4;     // lba 39:32
    uchar lba5;     // lba 47:40
    uchar featexp;  // feature register 15:8
    uchar count;    // sector count 7:0
    uchar countexp; // sector count 15:8
    uchar rsv1;
    uchar control;  // control register
    uint  rsv2;
};

typedef struct FisRegisterH2D FisRegisterH2D;


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
static CommandTable cmdtbl __attribute__((aligned(128)));
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

static void
checkalign(void *a, int alignment, char *msg)
{
    uintptr aa = (uintptr)a;

    if (aa & (alignment-1))
        panic(msg);
}


static int
findslot(Port *port)
{
    uint slots;
    int i;

    slots = port->sact | port->ci;

    for (i = 0; i < NCMD; i++) {
        if ((slots & 1) == 0)
            return i;
        slots >>= 1;
    }

    panic("ahci findslot");
    return -1;
}

static CommandHeader *
getcmdlist(Port *port)
{
    return (CommandHeader*)(((ulong)port->clbu << 32) + port->clb);
}

static void
mkprd(PhysicalRegionDescriptor *prd, uintptr addr, uint bytes)
{
    if (bytes > 4*MB)
        panic("mkprd");

    prd->dba = (uint)addr;
    if (hba.dma64)
        prd->dbau = (uint)(addr >> 32);

    prd->dbc = bytes - 1; // zero indexed
    prd->i = 1;
}

void
ahciread(uchar *addr, ulong lba, ushort sectcount)
{
    int slot, i;
    ushort sectleft;
    Port *port;
    CommandHeader *cmdhdr;
    uintptr addri;

    addri = (uintptr)addr;

    if (lba > ATA_MAXLBA48)
        panic("ahciread - maxlba");

    checkalign(addr, 2, "ahciread - addr align");

    if (!hba.dma64 && addri >= 4*GB)
        panic("ahciread - 64 bit start");
    if (!hba.dma64 && addri + sectcount*ATA_SECTSIZE >= 4*GB)
        panic("ahciread - 64 bit end");

    port = &hba.base->ports[0];
    port->is = 0xFFFFFFFF;      // clear interrupt flags

    slot = findslot(port);
    cmdhdr = &getcmdlist(port)[slot];

    cmdhdr->cfl = sizeof(FisRegisterH2D)/sizeof(uint);
    cmdhdr->w = 0; // read
    cmdhdr->prdtl = (sectcount*ATA_SECTSIZE + PRDSIZE - 1) / PRDSIZE; // round up to the nearest 4MB

    if (cmdhdr->prdtl > NPRD)
        panic("ahciread - prdtl");

    // WARING! ahciread relies on sizeof(CommandTable). If we ever
    // dynamically allocate Command Tables, we must change ahciread!
    memzero(&cmdtbl, sizeof(CommandTable));

    sectleft = sectcount;
    for (i = 0; i < cmdhdr->prdtl - 1; i++) {
        mkprd(&cmdtbl.prdt[i], addri, 4*MB);
        addri += 4*MB;
        sectleft -= 4*MB/ATA_SECTSIZE;
    }
    mkprd(&cmdtbl.prdt[i], addri, sectleft * ATA_SECTSIZE);

    FisRegisterH2D *cmdfis = (FisRegisterH2D *)(&cmdtbl.cfis);

    cmdfis->type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;
    cmdfis->command = ATA_CMD_READ_DMA_EXT;

    cmdfis->lba0 = (uchar)lba;
    cmdfis->lba1 = (uchar)(lba >> 8);
    cmdfis->lba2 = (uchar)(lba >> 16);
    cmdfis->device = ATA_LBA_MODE;

    cmdfis->lba3 = (uchar)(lba >> 24);
    cmdfis->lba4 = (uchar)(lba >> 32);
    cmdfis->lba5 = (uchar)(lba >> 40);

    cmdfis->count = (uchar)(sectcount);
    cmdfis->countexp = (uchar)(sectcount >> 8);

    while (port->tfd & (ATA_ST_BSY | ATA_ST_DRQ))
        ;

    port->ci = 1<<slot; // issue the command!

    // wait for the command to finish
    for (;;) {
        // break on complete
        if ((port->ci & (1<<slot)) == 0)
            break;

        // error bit set in ata status register
        if (port->is & PORT_IS_TFES)
            panic("ahciread - error 1");
    }

    // check again. no idea if this is necessary, but it's in an osdev example (of dubious quality)
    if (port->is & PORT_IS_TFES)
        panic("ahciread - error 2");
}


static HbaMemory *
abar(PciFunction *f)
{
    return (HbaMemory *)(ulong)(pcibar(f, 5) & 0xFFFFFFF0);
}

/*
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
*/

static int
popcount(ulong x)
{
    int count;
    for (count = 0; x; count++) {
        x &= x - 1; // clear least significant non-zero bit
    }
    return count;
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
    port->cmd &= ~PORT_CMD_FRE;

    while (port->cmd & (PORT_CMD_FR | PORT_CMD_CR))
        ;
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

    pcieach(findahci);

    if (ahcifound) {
        base = hba.base = abar(&hba.pci);
        hba.nports = popcount(base->pi);
        hba.nslots = ((base->cap >> 8) & 0x1F) + 1;
        hba.dma64  = base->cap & CAP_S64A;

        portinit(&base->ports[0], cmdlist, &fisstorage);
    }

    return ahcifound;
}
