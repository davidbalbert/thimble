#include "asm.h"
#include "mem.h"

.code16
.globl start

start:
    ljmp    $0x0000, $start1 # Get CS in a known state (0x0000)

start1:
    cli                     # Disable interrupts

    xor     %ax, %ax        # Zero out all segment registers (data and stack segment are
    mov     %ax, %ds        # particularly important).
    mov     %ax, %ss
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs

    cld                     # Clear direction flag: multi-byte string operations increment
                            # (rather than decrement) index registers.


    mov     $0x7BFE, %sp    # Set up the stack directly below the bootloader in
    mov     %sp, %bp        # memory. We can't set it to 0x7BFF, because the stack
                            # has to be 16-bit (2 byte) alligned. We're storing our
                            # paging structures at 0x1000, 0x2000, 0x3000, and 0x4000,
                            # so the stack can grow from 0x7BFE to 0x5000 without
                            # overwriting anything important (see
                            # http://wiki.osdev.org/Memory_Map_(x86)). That's more than
                            # 11 KiB of usable space, which should be plenty.

    # Enable the A20 gate for > 1 MB memory access
    # From xv6
seta20.1:
    inb     $0x64,%al               # Wait for not busy
    testb   $0x2,%al
    jnz     seta20.1

    movb    $0xd1,%al               # 0xd1 -> port 0x64
    outb    %al,$0x64

seta20.2:
    inb     $0x64,%al               # Wait for not busy
    testb   $0x2,%al
    jnz     seta20.2

    movb    $0xdf,%al               # 0xdf -> port 0x60
    outb    %al,$0x60

    # Enabeling long mode

    # Set up long mode page tables. We're going to identity map the first 2M of
    # memory and also map KERNBASE to the first 2M. We'll use 2MB pages.
    #
    # 0x1000 - PML4
    # 0x2000 - PDP for low addresses
    # 0x3000 - PD for low addresses
    # 0x4000 - PDP for high addresses
    # 0x5000 - PD for high addresses

    mov     $0x1000, %edi
    mov     %edi, %cr3      # Store the address of PML4T in CR3

    # Clear 0x1000 - 0x5FFF
    xor     %eax, %eax
    mov     $5120, %ecx
    rep stosl               # Writes 5120 (ECX) 32-bit length 0's (EAX) starting at 0x1000 (EDI)

    # First, identity map the first 2M. We mark each entry as present and
    # writable. We set the size bit in the pgdir entry to use 2MB pages.
    mov     $0x1000, %eax
    mov     $0x2000, %edx
    or      $(PTE_P|PTE_W), %edx
    mov     %edx, (%eax)

    mov     $0x2000, %eax
    mov     $0x3000, %edx
    or      $(PTE_P|PTE_W), %edx
    mov     %edx, (%eax)

    mov     $0x3000, %eax
    movw    $(0|PTE_P|PTE_W|PTE_PS), (%eax)

    # Then map KERNBASE
    mov     $0x1000, %eax
    mov     $0x4000, %edx
    or      $(PTE_P|PTE_W), %edx
    mov     $(KERNBASE>>39 & 0x1FF), %ecx
    mov     %edx, (%eax, %ecx, 8)

    mov     $0x4000, %eax
    mov     $0x5000, %edx
    or      $(PTE_P|PTE_W), %edx
    mov     $(KERNBASE>>30 & 0x1FF), %ecx
    mov     %edx, (%eax, %ecx, 8)

    mov     $0x5000, %eax
    mov     $(0|PTE_P|PTE_W|PTE_PS), %edx
    mov     $(KERNBASE>>21 & 0x1FF), %ecx
    mov     %edx, (%eax, %ecx, 8)

    # Enable PAE
    mov     %cr4, %eax
    or      $(1 << 5), %eax
    mov     %eax, %cr4

    # Enable Long Mode in the EFER Machine Specific Register
    mov     $0xC0000080, %ecx   # The address of the EFER MSR
    rdmsr                       # Stores the 64-bit value of EFER into EDX:EAX
    or      $(1 << 8), %eax     # Set the LM bit
    wrmsr

    # Enable paging (31) and protected mode (0)
    mov     %cr0, %eax
    or      $(1 << 31 | 1 << 0), %eax
    mov     %eax, %cr0

    lgdt    gdtdesc         # Load the GDT

    ljmp    $0x8, $start64  # Far jump into protected mode! 0x8 is the selector
                            # of the selector of the code segment descriptor, which
                            # gets stored in CS.

.code64
start64:
    mov     $0x10, %ax      # 0x10 is the data segment descriptor selector.
    mov     %ax, %ds
    mov     %ax, %ss
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs

    call    bootmain        # Into C

    # bootmain should never return, but just in case...
done:
    hlt
    jmp     done

# The GDT should be alligned on an 8-byte boundary. See section 3.5.1, of
# volume 3A of the Intel 64 and IA-32 Architectures Software Developer's
# Manual.
.balign 8
gdt:
    GDT_ENTRY(0, 0, 0, 0)
    GDT_ENTRY(0, 0, 0b10011010, 0b1010)     # Code segment
    GDT_ENTRY(0, 0, 0b10010010, 0)          # Data segment
    # We need a TSS segment here, but I don't know what that is yet.

gdtdesc:
    .word   (gdtdesc - gdt - 1)     # sizeof(gdt) - 1
    .long   gdt
