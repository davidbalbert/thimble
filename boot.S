#include "asm.h"

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
    call    a20waitwrite
    mov     $0xd0, %al      # Tell the controller we want to read the Controller Output Port
    out     %al, $0x64

    call    a20waitread
    in      $0x60, %al      # Read the value of the COP
    or      $2, %al         # Set the A20 bit
    push    %ax
    
    call    a20waitwrite
    mov     $0xd1, %al      # Now we want to write the output port
    out     %al, $0x64

    call    a20waitwrite
    pop     %ax
    out     %al, $0x60      # Write the new COP value
    
    jmp     a20done

a20waitwrite:
    in      $0x64, %al      # Wait for PS2 controller to be ready by making sure bit 1
    test    $2, %al         # of the status register is low.
    jnz     a20waitwrite
    ret

a20waitread:
    in      $0x64, %al
    test    $1, %al         # status register bit 0 is high when we're allowed to read
    jz      a20waitread
    ret

a20done:

    # Enabeling Long Mode

    # Set up long mode page tables. We're going to identity map the first two
    # megabytes of memory.

    mov     $0x1000, %edi
    mov     %edi, %cr3      # Store the address of PML4T in CR3

    # Clear 0x1000 - 0x4FFF
    xor     %eax, %eax
    mov     $4096, %ecx
    rep stosl               # Writes 4096 (ECX) 32-bit length 0's (EAX) starting at 0x1000 (EDI)

    # Map the page tables
    mov     %cr3, %edi
    movl    $0x2003, (%edi) # Point PML4T[0] to a PDPT at 0x2000, marked as read/write and present (bits 2 and 1).
    add     $0x1000, %edi
    movl    $0x3003, (%edi) # Point PDPT[0] to a PDT at 0x3000, marked as read/write and present
    add     $0x1000, %edi
    movl    $0x4003, (%edi) # Point PDP[0] to a PT at 0x4000, marked as read/write and present
    add     $0x1000, %edi

    # Identity map all 512 entries in PML4T[0]->PDPT[0]->PDT[0]->PT. This is 2 MiB.
    mov     $0x3, %ebx
    mov     $512, %ecx

map_entry:
    movl    %ebx, (%edi)    # Map the address of the current page (with r/w and present)
    add     $0x1000, %ebx   # Calculate the address of the next page
    add     $8, %edi        # Move to next PTE
    loop    map_entry       # Jumps to map_entry and decrements ECX unless ECX == 1

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

    ljmp    $0x8, $start64   # Far jump into long mode! 0x8 is the selector of the selector of the code segment descriptor, which gets stored in CS.

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
