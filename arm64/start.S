.global _start
_start:
    mrs x1, mpidr_el1

    and x1, x1, #0xFF
    cmp x1, #0
    beq boot_cpu0

loop:
    wfi
    b loop

boot_cpu0:
    ldr x0, =STACKTOP
    mov sp, x0
    bl main

halt:
    wfi
    b halt
