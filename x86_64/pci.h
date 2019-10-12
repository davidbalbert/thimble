struct PciFunction {
    ushort vendorid;
    uchar bus;
    uchar dev;
    uchar func;
    uchar class;
    uchar subclass;
};

typedef struct PciFunction PciFunction;

#define PCI_C_STORAGE 0x1

#define PCI_SC_AHCI   0x6
