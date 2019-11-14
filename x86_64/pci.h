struct PciFunction {
    u16 vendorid;
    byte bus;
    byte dev;
    byte func;
    byte class;
    byte subclass;
};

typedef struct PciFunction PciFunction;

#define PCI_C_STORAGE 0x1

#define PCI_SC_AHCI   0x6
