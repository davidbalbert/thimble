#include "asm.h"
#include "mem.h"

#include "stage2size.h"

#define STAGE2 0x7E00

.code16
.globl _start

_start:
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


    mov     $_start, %sp     # Set up the bootloader's stack directly below the
    mov     %sp, %bp        # bootloader in memory. We're storing our paging structures
                            # at 0x1000, 0x2000, 0x3000, and 0x4000, so the stack can
                            # grow from 0x7C00 to 0x5000 without overwriting anything
                            # important (see http://wiki.osdev.org/Memory_Map_(x86)).
                            # The kernel will use a different stack (see entry.S).

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


    # Load stage 2 from disk
    xor     %ax, %ax
    mov     $0x42, %ah      # Subroutine identifier for LBA disk reads?
    mov     $0x80, %dl      # C drive
    mov     $dap, %si       # Address of DAP
    int     $0x13           # Call the BIOS


    # Enabeling long mode

    # Set up long mode page tables. We're going to identity map the first 4 GB
    # of memory and also map KERNBASE to the first 4 GB. We'll use 1 GB pages.
    # We're mapping in so much memory because the bootloader needs to access
    # the AHCI controller which is a memory mapped PCI device that lives
    # between the 3rd and 4th GB of physical memory.
    #
    # 0x1000 - PML4
    # 0x2000 - PDP

    mov     $0x1000, %edi
    mov     %edi, %cr3      # Store the address of PML4 in CR3

    # Clear 0x1000 - 0x2FFF
    xor     %eax, %eax
    mov     $(0x2000 / 4), %ecx
    rep stosl               # Zeroes (EAX) out 0x2000 (ECX) bytes in 4 byte increments starting at 0x1000 (EDI)

    # First, identity map the first 4 GB. We mark each entry as present
    # and writable.
    #
    mov     $0x1000, %eax
    mov     $0x2000, %edx
    or      $(PTE_P|PTE_W), %edx
    mov     %edx, (%eax)

    # Then map KERNBASE. We can point KERNBASE at the same PDP as the low
    # memory because the low and high memory have the same lower 39 bits.
    mov     $(KERNBASE>>39 & 0x1FF), %ecx
    mov     %edx, (%eax, %ecx, 8)

    # Now map in the first GB
    mov     $0x2000, %eax
    mov     $(PTE_P|PTE_W|PTE_PS), %edx
    mov     %edx, (%eax)

    # Second GB
    add     $8, %eax
    add     $0x40000000, %edx
    mov     %edx, (%eax)

    # Third GB
    add     $8, %eax
    add     $0x40000000, %edx
    mov     %edx, (%eax)

    # Fourth GB
    add     $8, %eax
    add     $0x40000000, %edx
    mov     %edx, (%eax)

    # Enable PAE and PSE
    mov     %cr4, %eax
    or      $(1 << 5), %eax # Physical Address Extension
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

    mov     $((STAGE2SIZE + 1) * 512), %rdi   # Stage 2 needs to know where to start loading the kernel from.
    call    STAGE2

    # stage2 should never return, but just in case...
done:
    hlt
    jmp     done

# Disk Address Packet (DAP)
.balign 4
dap:
    .byte   16             # DAP size
    .byte   0              # Always zero
    .word   STAGE2SIZE     # Number of sectors to read
    .word   STAGE2         # Destination offset
    .word   0              # Destination real mode segment
    .long   1              # Starting LBA
    .long   0              # Upper 16 bits of 48-bit LBA

# The GDT should be alligned on an 8-byte boundary. See section 3.5.1, of
# volume 3A of the Intel 64 and IA-32 Architectures Software Developer's
# Manual.
.balign 8
gdt:
    GDT_ENTRY(0, 0, 0, 0)
    GDT_ENTRY(0, 0, 0b10011010, 0b0010)     # Code segment
    GDT_ENTRY(0, 0, 0b10010010, 0)          # Data segment

gdtdesc:
    .word   (gdtdesc - gdt - 1)     # sizeof(gdt) - 1
    .quad   gdt
