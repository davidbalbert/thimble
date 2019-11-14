// ahci driver - is part of the boot loader and uses physical addresses
//
// How to modify this to be shared with the kernel:
// - pull out all static data into bootahci.c
//      - when the bootloader calls functions in ahci.c, use p2v on any pointers passed in
// - use p2v inside abar
// - when setting pointer fields in either the HBA registers or any of the
//   system data structures, use v2p.
//
// TODO: we're not doing anything to make sure caches stay coherent while the
// AHCI controller writes to main memory. I've seen different resources on the
// internet say different things about whether x86 maintains PCI DMA cache
// coherency. For now, it boots in QEMU, so we'll call it a day.

#include "u.h"

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
    u32 clb;       // command list base address, 1K-byte aligned
    u32 clbu;      // command list base address upper 32 bits
    u32 fb;        // FIS base address, 256-byte aligned
    u32 fbu;       // FIS base address upper 32 bits
    u32 is;        // interrupt status
    u32 ie;        // interrupt enable
    u32 cmd;       // command and status
    u32 rsv0;      // Reserved
    u32 tfd;       // task file data
    u32 sig;       // signature
    u32 ssts;      // SATA status (SCR0:SStatus)
    u32 sctl;      // SATA control (SCR2:SControl)
    u32 serr;      // SATA error (SCR1:SError)
    u32 sact;      // SATA active (SCR3:SActive)
    u32 ci;        // command issue
    u32 sntf;      // SATA notification (SCR4:SNotification)
    u32 fbs;       // FIS-based switch control
    u32 rsv1[11];
    u32 vendor[4]; // vendor specific
};

typedef volatile struct Port Port;

struct HbaMemory {
    u32 cap;       // Host capability
    u32 ghc;       // Global host control
    u32 is;        // Interrupt status
    u32 pi;        // Port implemented
    u32 vs;        // Version
    u32 ccc_ctl;   // Command completion coalescing control
    u32 ccc_pts;   // Command completion coalescing ports
    u32 em_loc;    // Enclosure management location
    u32 em_ctl;    // Enclosure management control
    u32 cap2;      // Host capabilities extended
    u32 bohc;      // BIOS/OS handoff control and status

    byte rsv[116];

    byte vendor[96];   // Vendor specific registers

    Port ports[0];   // Between 1 and 32 ports
};

typedef volatile struct HbaMemory HbaMemory;

struct CommandHeader {
    u32 cfl:5;     // Command FIS length
    u32 a:1;       // ATAPI
    u32 w:1;       // Write
    u32 p:1;       // Prefetchable

    u32 r:1;       // Reset
    u32 b:1;       // BIST
    u32 c:1;       // Clear busy upon R_OK
    u32 rsv0:1;    // Reserved
    u32 pmp:4;     // Port multiplier port

    u16 prdtl;           // Physical region descriptor table length in entries
    volatile u32 prdbc;    // Physical region descriptor byte count transferred. Inititalize to 0.

    u32 ctba;      // Command table descriptor base address, 128-byte aligned
    u32 ctbau;     // Command table descriptor base address upper 32 bits

    u32 rsv1[4];
};

typedef struct CommandHeader CommandHeader;


#define PRDSIZE (4*MB) // max byte count in a PRDT entry

struct PhysicalRegionDescriptor
{
    u32 dba;       // Data base address
    u32 dbau;      // Data base address upper 32 bits
    u32 rsv0;      // Reserved

    u32 dbc:22;    // Byte count, 4M max
    u32 rsv1:9;    // Reserved
    u32 i:1;       // Interrupt on completion
};

typedef struct PhysicalRegionDescriptor PhysicalRegionDescriptor;


#define NPRD 8

struct CommandTable
{
    byte cfis[64];    // Command FIS
    byte acmd[16];    // ATAPI command, 12 or 16 bytes
    byte rsv[48];

    // WARING! ahciread relies on sizeof(CommandTable). If we ever
    // dynamically allocate Command Tables, we must change ahciread!
    PhysicalRegionDescriptor prdt[NPRD];   // can contain 0 to 65535 entries
};

typedef struct CommandTable CommandTable;


#define FIS_TYPE_REG_H2D 0x27

struct FisRegisterH2D {
    byte type;
    u32  rsv0:7;
    u32  c:1;      // 1 - command block update, 0 = device control block update
    byte command;  // command register
    byte feat;     // feature register 7:0
    byte lba0;     // lba 7:0
    byte lba1;     // lba 15:8
    byte lba2;     // lba 23:16
    byte device;   // device register
    byte lba3;     // lba 31:24
    byte lba4;     // lba 39:32
    byte lba5;     // lba 47:40
    byte featexp;  // feature register 15:8
    byte count;    // sector count 7:0
    byte countexp; // sector count 15:8
    byte rsv1;
    byte control;  // control register
    u32  rsv2;
};

typedef struct FisRegisterH2D FisRegisterH2D;


struct ReceivedFisStorage {
    byte   dsfis[28];      // dma setup fis
    byte   pad0[4];

    byte   psfis[20];      // pio setup (device to host) fis
    byte   pad1[12];

    byte   rfis[20];       // device to host register fis
    byte   pad2[4];

    byte   sdbfis[8];      // set device bits fis

    byte   ufis[64];       // unknown fis

    // the spec's diagram has 95 bytes, but osdev has
    // 96 and I'd rather waste a byte than have a random
    // byte somewhere else overwritten.
    byte   reserved[96];
};

typedef volatile struct ReceivedFisStorage ReceivedFisStorage;



static struct {
    PciFunction pci;
    HbaMemory *base;
    int nports;         // number of ports
    int nslots;         // number of command slots per port
    int dma64;          // bool - supports 64-bit physical addresses for DMA
} hba; // Host bus adapter

#define NCMD 32

static CommandHeader cmdlist[NCMD] __attribute__((aligned(1*KB)));
static CommandTable cmdtbls[NCMD] __attribute__((aligned(128)));
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
    u32 slots;
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
    return (CommandHeader *)(((u64)port->clbu << 32) + port->clb);
}

static CommandTable *
getcmdtbl(CommandHeader *cmdhdr)
{
    return (CommandTable *)(((u64)cmdhdr->ctbau << 32) + cmdhdr->ctba);
}

static void
mkprd(PhysicalRegionDescriptor *prd, uintptr addr, u32 bytes)
{
    if (bytes > 4*MB)
        panic("mkprd");

    prd->dba = (u32)addr;
    if (hba.dma64)
        prd->dbau = (u32)(addr >> 32);

    prd->dbc = bytes - 1; // zero indexed
    prd->i = 1;
}

void
ahciread(byte *addr, u64 lba, u16 sectcount)
{
    int slot, i;
    u16 sectleft;
    uintptr addri;
    Port *port;
    CommandHeader *cmdhdr;
    CommandTable *cmdtbl;

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
    cmdtbl = getcmdtbl(cmdhdr);

    cmdhdr->cfl = sizeof(FisRegisterH2D)/sizeof(u32);
    cmdhdr->w = 0; // read
    cmdhdr->prdtl = (sectcount*ATA_SECTSIZE + PRDSIZE - 1) / PRDSIZE; // round up to the nearest 4MB

    if (cmdhdr->prdtl > NPRD)
        panic("ahciread - prdtl");

    // WARING! ahciread relies on sizeof(CommandTable). If we ever
    // dynamically allocate Command Tables, we must change ahciread!
    memzero(cmdtbl, sizeof(CommandTable));

    sectleft = sectcount;
    for (i = 0; i < cmdhdr->prdtl - 1; i++) {
        mkprd(&cmdtbl->prdt[i], addri, 4*MB);
        addri += 4*MB;
        sectleft -= 4*MB/ATA_SECTSIZE;
    }
    mkprd(&cmdtbl->prdt[i], addri, sectleft * ATA_SECTSIZE);

    FisRegisterH2D *cmdfis = (FisRegisterH2D *)(&cmdtbl->cfis);

    cmdfis->type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;
    cmdfis->command = ATA_CMD_READ_DMA_EXT;

    cmdfis->lba0 = (byte)lba;
    cmdfis->lba1 = (byte)(lba >> 8);
    cmdfis->lba2 = (byte)(lba >> 16);
    cmdfis->device = ATA_LBA_MODE;

    cmdfis->lba3 = (byte)(lba >> 24);
    cmdfis->lba4 = (byte)(lba >> 32);
    cmdfis->lba5 = (byte)(lba >> 40);

    cmdfis->count = (byte)(sectcount);
    cmdfis->countexp = (byte)(sectcount >> 8);

    while (port->tfd & (ATA_STS_BSY | ATA_STS_DRQ))
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
    return (HbaMemory *)(u64)(pcibar(f, 5) & 0xFFFFFFF0);
}

/*
static char *
version(u32 vs) {
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
speed(u32 cap)
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
popcount(u64 x)
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
portinit(Port *port, CommandHeader *cl, CommandTable *ctlist, ReceivedFisStorage *fisbase)
{
    uintptr cll, fisbasel, ctlistl;
    int i;

    checkalign(cl, 1*KB, "portinit - cl align");
    checkalign((void *)fisbase, 256, "portinit - fisbase align");

    cll = (uintptr)cl;
    fisbasel = (uintptr)fisbase;
    ctlistl = (uintptr)ctlist;

    if (!hba.dma64 && cll >= 4*GB)
        panic("portinit - cl");
    if (!hba.dma64 && fisbasel >= 4*GB)
        panic("portinit - fisbase");
    if (!hba.dma64 && ctlistl >= 4*GB)
        panic("portinit - ctlist");

    portstop(port);

    port->clb = (u32)cll;
    if (hba.dma64)
        port->clbu = (u32)(cll >> 32);

    port->fb = (u32)fisbasel;
    if (hba.dma64)
        port->fbu = (u32)(fisbasel >> 32);

    for (i = 0; i < NCMD; i++) {
        cl[i].prdtl = NPRD;
        cl[i].ctba = (u32)ctlistl;
        if (hba.dma64)
            cl[i].ctbau = (u32)(ctlistl >> 32);
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

        portinit(&base->ports[0], cmdlist, cmdtbls, &fisstorage);
    }

    return ahcifound;
}
