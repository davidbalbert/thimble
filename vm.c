#include "types.h"

#include "mem.h"
#include "proc.h"
#include "x86.h"


struct TSSDescriptor {
};
typedef struct TSSDescriptor TSSDescriptor;


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
    d->type = 0b0010;   // Data, R+W
    d->s = 1;           // application (code or data) segment
    d->dpl = dpl;
    d->p = 1;           // Always present
    d->avl = 0;         // For use by system
    d->l = 0;           // Always 0 for data segments
    d->db = 0;
    d->g = 0;
    d->base24_31 = 0;
}

void
seginit(void)
{
    codedesc(&cpu->gdt[SEG_KCODE], 0);
    datadesc(&cpu->gdt[SEG_KDATA], 0);
    codedesc(&cpu->gdt[SEG_UCODE], 3);
    datadesc(&cpu->gdt[SEG_UDATA], 3);

    lgdt(cpu->gdt, sizeof(cpu->gdt));
}
