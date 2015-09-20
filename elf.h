#define ELF_MAGIC 0x464C457F

typedef struct {
    uint magic;
    uchar ident[12];

    ushort type;
    ushort machine;
    uint version;
    ulong entry;
    ulong phoff;
    ulong shoff;
    uint flags;
    ushort ehsize;
    ushort phentsize;
    ushort phnum;
    ushort shentsize;
    ushort shnum;
    ushort shstrndx;
} ElfHeader;

typedef struct {
    uint type;
    uint flags;
    ulong offset;
    ulong vaddr;
    ulong paddr;
    ulong filesz;
    ulong memsz;
    ulong align;
} ElfProgHeader;
