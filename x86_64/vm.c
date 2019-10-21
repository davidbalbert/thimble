#include "u.h"

#include "defs.h"
#include "mem.h"
#include "proc.h"
#include "x86.h"

static Pte *kpgmap;

// Makes a 64 bit code segment descriptor for the given dpl
static void
codedesc(SegmentDescriptor *d, uchar dpl)
{
    d->limit0_15 = 0;
    d->base0_15 = 0;
    d->base16_23 = 0;
    d->type = 0b1010;   // Code, R+X
    d->s = 1;           // application (code or data) segment
    d->dpl = dpl;
    d->p = 1;           // Always present
    d->avl = 0;         // For use by system
    d->l = 1;           // 64 bit code segment
    d->db = 0;          // Should always be zero for 64 bit code segments
    d->g = 0;
    d->base24_31 = 0;
}

static void
datadesc(SegmentDescriptor *d, uchar dpl)
{
    d->limit0_15 = 0;
    d->base0_15 = 0;
    d->base16_23 = 0;
    d->type = 0b0010;   // data, R+W
    d->s = 1;           // application (code or data) segment
    d->dpl = dpl;
    d->p = 1;           // always present
    d->avl = 0;         // for use by system
    d->l = 0;           // always 0 for data segments
    d->db = 0;
    d->g = 0;
    d->base24_31 = 0;
}

static void
tsdesc(TaskStateDescriptor *d, TaskState *ts, uint size)
{
    ulong base = (ulong)ts;
    uint limit = size - 1;      // limits in the CPU seem to be one less than the size

    d->limit0_15 = limit;
    d->base0_15 = base;
    d->base16_23 = base >> 16;
    d->type = 0b1001;           // 64 bit TSS
    d->s = 0;                   // system
    d->dpl = 0;
    d->p = 1;                   // present
    d->limit16_19 = limit >> 16;
    d->avl = 0;                 // for use by system
    d->l = 0;                   // zero for TSS
    d->db = 0;                  // zero for TSS
    d->g = 0;                   // byte granularity for limit
    d->base24_31 = base >> 24;
    d->base32_63 = base >> 32;
    d->reserved = 0;            // zero out the "type" bits of the higher 8 bytes
}

// virtual address -> PML4 index
static int
pmx(void *va)
{
    uintptr a = (uintptr)va;
    return (a >> PML4SHIFT) & 0x1FF;
}

// virtual address -> PDP index
static int
pdpx(void *va)
{
    uintptr a = (uintptr)va;
    return (a >> PDPSHIFT) & 0x1FF;
}

// virtual address -> PD index
static int
pdx(void *va)
{
    uintptr a = (uintptr)va;
    return (a >> PDSHIFT) & 0x1FF;
}

// virtual address -> PT index
static int
ptx(void *va)
{
    uintptr a = (uintptr)va;
    return (a >> PTSHIFT) & 0x1FF;
}

// paging structure entry -> physical address
static uintptr
pte_addr(Pte entry)
{
    return entry & ~0xFFF;
}

static int
pte_flags(Pte entry)
{
    return entry & 0xFFF;
}

static ulong *
pgmapget(ulong *table, int offset, int alloc)
{
    ulong *innertab;
    ulong *entry = &table[offset];

    if (*entry & PTE_P) {
        innertab = (ulong *)p2v(pte_addr(*entry));
    } else {
        if (!alloc || (innertab = kalloc()) == nil)
            return nil;

        memzero(innertab, PGSIZE);

        // xv6 gives all the permissions, but where does
        // it restrict it?
        *entry = v2p(innertab) | PTE_W | PTE_U | PTE_P;
    }

    return innertab;
}

static Pte *
walkpgmap(Pte *pgmap, void *va, int alloc)
{
    Pte *pdirpt;
    Pte *pgdir;
    Pte *pgtab;

    pdirpt = pgmapget(pgmap, pmx(va), alloc);

    if (pdirpt == nil)
        return nil;

    pgdir = pgmapget(pdirpt, pdpx(va), alloc);

    if (pgdir == nil)
        return nil;

    pgtab = pgmapget(pgdir, pdx(va), alloc);

    if (pgtab == nil)
        return nil;

    return &pgtab[ptx(va)];
}

// user virtual address to kernel address
void *
uva2ka(Pte *pgmap, void *addr)
{
    Pte *pte = walkpgmap(pgmap, addr, 0);

    if (pte == nil)
        return nil;
    if ((*pte & PTE_P) == 0)
        return nil;
    if ((*pte & PTE_U) == 0)
        return nil;

    uintptr pg = (uintptr)p2v(pte_addr(*pte));
    uintptr a = pg | ((uintptr)addr & 0xFFF);

    return (void *)a;
}

void
clearpteu(Pte *pgmap, void *addr)
{
    Pte *pte = walkpgmap(pgmap, addr, 0);
    if (pte == nil)
        panic("clearpteu");

    *pte &= ~PTE_U;
}

static void
checkalign(void *a, int alignment, char *msg)
{
    uintptr aa = (uintptr)a;

    if (aa & (alignment-1))
        panic(msg);
}

static int
mappages(Pte *pgmap, void *va, usize size, uintptr pa, int perm)
{
    char *a, *last;
    Pte *pte;

    checkalign((void *)pa, PGSIZE, "mappages - physical addr not page aligned");

    a = pgfloor(va);
    last = pgfloor((void *)((uintptr)va + size - 1));

    for (;;) {
        pte = walkpgmap(pgmap, a, 1);

        if (pte == nil)
            return -1;
        if (*pte & PTE_P)
            panic("remap, va: 0x%x, pa: 0x%x", va, pa);

        *pte = pa | perm | PTE_P;

        if (a == last)
            break;
        a += PGSIZE;
        pa += PGSIZE;
    }

    return 0;
}

// Memeory layout borrowed heavily from xv6, but adapted for 64 bit:
//
//      0..USERTOP: user memory (canonical lower half; not included in kmap below)
//
//      KERNBASE..KERNBASE+EXTMEM -> 0..EXTMEM: I/O space (e.g. VGA), bootloader stack, etc.
//      KERNBASE+EXTMEM..data -> EXTMEM..v2p(data): kernel text and rodata. Read-only.
//      data..KERNBASE+PHYSTOP -> v2p(data)..PHYSTOP: kernel data and 1-1 mapping with free physical memory
//      KERNBASE+DEVSPACE..KERNBASE+DEVTOP -> DEVSPACE..DEVTOP: memory mapped devices

typedef struct Kmap Kmap;
static struct Kmap {
    void *addr;
    uintptr phys_start;
    uintptr phys_end;
    int perm;
} kmap[] = {
    {(void *)KERNBASE,           0,         EXTMEM,    PTE_W}, // memory mapped devices
    {(void *)KERNBASE+EXTMEM,    EXTMEM,    V2P(data), 0},     // Kernel read only data
    {(void *)data,               V2P(data), PHYSTOP,   PTE_W}, // kernel data + physical pages
    {(void*)(KERNBASE+DEVSPACE), DEVSPACE,  DEVTOP,    PTE_W}, // more devices
};


Pte *
setupkvm(void)
{
    Pte *pgmap = kalloc();
    Kmap *k;

    if (pgmap == nil)
        return nil;

    memzero(pgmap, PGSIZE);

    for (k = kmap; k < &kmap[nelem(kmap)]; k++) {
        if (mappages(pgmap, k->addr, k->phys_end-k->phys_start, k->phys_start, k->perm) < 0)
            return nil;
    }

    return pgmap;
}

void
switchkvm(void)
{
    lcr3(v2p(kpgmap));
}

void
kvmalloc(void)
{
    if ((kpgmap = setupkvm()) == nil)
        panic("kvmalloc");

    switchkvm();
}

usize
allocuvm(Pte *pgmap, usize oldsz, usize newsz)
{
    void *mem;
    usize a;

    if (newsz >= USERTOP)
        return 0;
    if (newsz < oldsz)
        return oldsz;

    a = (usize)pgceil((void *)oldsz);

    for (; a < newsz; a += PGSIZE) {
        mem = kalloc();
        if (mem == nil) {
            cprintf("allocuvm -- oom (should dealloc here)\n");
            return 0;
        }

        memzero(mem, PGSIZE);

        if (mappages(pgmap, (char *)a, PGSIZE, v2p(mem), PTE_W|PTE_U) < 0) {
            cprintf("allocuvm -- oom 2 (should dealloc here)\n");
            kfree(mem);
            return 0;
        }
    }

    return newsz;
}

void
loaduvm(Pte *pgmap, char *addr, uchar *data, ulong sz)
{
    Pte *pte;
    ulong i;
    usize n;
    char *pg;

    checkalign(addr, 4096, "loaduvm - addr not page aligned");

    for (i = 0; i < sz; i+= PGSIZE) {
        pte = walkpgmap(pgmap, addr+i, 0);
        if (pte == nil)
            panic("loaduvm - addr not mapped");

        pg = p2v(pte_addr(*pte));

        if (sz - i < PGSIZE)
            n = sz - i;
        else
            n = PGSIZE;

        memmove(pg, data + i, n);
    }
}

// Sets up virtual memory for process p
void
switchuvm(Proc *p)
{
    TaskStateDescriptor *d;

    pushcli();

    d = (TaskStateDescriptor *)&cpu->gdt[SEG_TSS];
    tsdesc(d, &cpu->ts, sizeof(cpu->ts));
    cpu->ts.rsp0 = (ulong)p->kstack + KSTACKSIZE;
    cpu->ts.iomapbase = 0xFFFF; // disable in/out in user space

    ltr(SEG_TSS << 3);
    if (p->pgmap == nil)
        panic("switchuvm - no pgmap");
    lcr3(v2p(p->pgmap));

    popcli();
}

Pte *
copyuvm(Pte *oldmap, usize sz)
{
    uintptr a;
    Pte *newmap;
    Pte *pte;
    uchar *oldmem, *newmem;
    uint flags;

    newmap = setupkvm();
    if (newmap == nil)
        return nil;

    for (a = 0; a < sz; a += PGSIZE) {
        pte = walkpgmap(oldmap, (void *)a, 0);
        if (pte == nil)
            panic("copyuvm - nil pte");
        if (!*pte & PTE_P)
            panic("copyuvm - page not present");

        oldmem = p2v(pte_addr(*pte));
        flags = pte_flags(*pte);

        newmem = kalloc();
        if (newmem == nil)
            goto bad;

        memmove(newmem, oldmem, PGSIZE);

        if (mappages(newmap, (void *)a, PGSIZE, v2p(newmem), flags) < 0)
            goto bad;
    }

    return newmap;

bad:
    freeuvm(newmap);
    return nil;
}

void
freeuvm(Pte *pgmap)
{
    cprintf("freeuvm not implemented yet!\n");
    // todo
}

void
seginit(void)
{
    codedesc(&cpu->gdt[SEG_KCODE], KERN_DPL);
    datadesc(&cpu->gdt[SEG_KDATA], KERN_DPL);
    datadesc(&cpu->gdt[SEG_UDATA], USER_DPL);
    codedesc(&cpu->gdt[SEG_UCODE], USER_DPL);

    lgdt(cpu->gdt, sizeof(cpu->gdt));
}