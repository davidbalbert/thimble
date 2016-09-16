#include "types.h"

#include "mem.h"
#include "proc.h"
#include "x86.h"

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

// Sets up virtual memory for process p
void
switchuvm(Proc *p)
{
    TaskStateDescriptor *d;

    d = (TaskStateDescriptor *)&cpu->gdt[SEG_TSS];
    tsdesc(d, &cpu->ts, sizeof(cpu->ts));
    cpu->ts.rsp0 = (ulong)p->kstack + KSTACKSIZE;

    ltr(SEG_TSS << 3);
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
