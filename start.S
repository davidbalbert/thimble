#include "mem.h"

.code64
.globl _start

# _start is the ELF entry point, but we haven't yet mapped
# KERNBASE, so we'll jump to the physical address instead.
_start = V2P(start)

start:
    # When we boot, the first two megabytes are identity mapped
    # using a temporary pagedir hierarchy set up by the bootloader
    # (a prerequisite for entering long mode). We are going build
    # a new temporary pagedir hierarchy that identity maps the
    # first two megabytes as well as mapping KERNBASE -
    # KERNBASE+2MB to the first 2 megabytes. We use 2MB pages to
    # make things a bit easier.

    # First, identity map the first 2M
    mov     $(V2P(bootpml4)), %rax
    mov     $(V2P(bootpdplow)), %rdx
    or      $(PTE_P|PTE_W), %rdx
    mov     %rdx, (%rax)

    mov     $(V2P(bootpdplow)), %rax
    mov     $(V2P(bootpgdirlow)), %rdx
    or      $(PTE_P|PTE_W), %rdx
    mov     %rdx, (%rax)

    mov     $(V2P(bootpgdirlow)), %rax
    movq    $(0|PTE_P|PTE_W|PTE_PS), (%rax)

    # Then map KERNBASE
    mov     $(V2P(bootpml4)), %rax
    mov     $(V2P(bootpdphigh)), %rdx
    or      $(PTE_P|PTE_W), %rdx
    mov     $(KERNBASE>>39 & 0x1FF), %rcx
    mov     %rdx, (%rax, %rcx, 8) 

    mov     $(V2P(bootpdphigh)), %rax
    mov     $(V2P(bootpgdirhigh)), %rdx
    or      $(PTE_P|PTE_W), %rdx
    mov     $(KERNBASE>>30 & 0x1FF), %rcx
    mov     %rdx, (%rax, %rcx, 8)

    mov     $(V2P(bootpgdirhigh)), %rax
    mov     $(0|PTE_P|PTE_W|PTE_PS), %rdx
    mov     $(KERNBASE>>21 & 0x1FF), %rcx
    mov     %rdx, (%rax, %rcx, 8)

    # Load the new paging structures
    mov     $(V2P(bootpml4)), %rax
    mov     %rax, %cr3

    # Up until now, the kernel has been running in low memory
    # (0x100000). Here we use an indirect jump to get into high
    # memory (0xFFFF800000100000).
    movabs  $main, %rax
    jmp     *%rax

stop:
    hlt
    jmp stop
