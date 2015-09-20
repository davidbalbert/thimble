#include "mem.h"

.code64
.globl _start

# _start is the ELF entry point, but we haven't yet mapped
# KERNBASE, so we'll jump to the physical address instead.
_start = V2P(start)

start:
    # When we boot, the first two megabytes are identity mapped
    # using a temporary pagedir hierarchy in the bootloader. We are
    # going build a new temporary pagedir hierarchy that identity
    # maps the first two megabytes as well as mapping KERNBASE -
    # KERNBASE+2MB to the first 2 megabytes.

    mov     $(V2P(bootpml4)), %rax
    mov     $(V2P(bootpdplow)), %rcx
    or      $(PTE_P|PTE_W), %rcx
    mov     %rcx, (%rax)
    mov     $(KERNBASE>>39 & 0x1FF), %rdx
    mov     $(V2P(bootpdphigh)), %rcx
    or      $(PTE_P|PTE_W), %rcx
    mov     %rcx, (%rax, %rdx, 1)

    mov     $(V2P(bootpdplow)), %rax
    mov     $(V2P(bootpgdirlow)), %rcx
    or      $(PTE_P|PTE_W), %rcx
    mov     %rcx, (%rax)

    mov     $(V2P(bootpgdirlow)), %rax
    movq    $(0|PTE_P|PTE_W|PTE_PS), (%rax)

    mov     $(V2P(bootpdphigh)), %rax
    mov     $(KERNBASE>>30 & 0x1FF), %rdx
    mov     $(V2P(bootpgdirhigh)), %rcx
    or      $(PTE_P|PTE_W), %rcx
    mov     %rcx, (%rax, %rdx, 1)

    mov     $(V2P(bootpgdirhigh)), %rax
    mov     $(KERNBASE>>21 & 0x1FF), %rdx
    movq    $(0|PTE_P|PTE_W|PTE_PS), (%rax, %rdx, 1)

    mov     $(V2P(bootpml4)), %rax
    mov     %rax, %cr3

    call    kmain

stop:
    hlt
    jmp stop
