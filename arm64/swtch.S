// void swtch(Registers **from, Registers *to)
//   x0 - from
//   x1 - to

.global swtch
swtch:
    stp x29, x30, [sp, #-16]!
    stp x27, x28, [sp, #-16]!
    stp x25, x26, [sp, #-16]!
    stp x23, x24, [sp, #-16]!
    stp x21, x22, [sp, #-16]!
    stp x19, x20, [sp, #-16]!

    // SP now points to the old saved registers. Store it in *from.
    mov x19, sp
    str x19, [x0]

    // Our new stack pointer is the same as to
    mov sp, x1

    ldp x19, x20, [sp], #16
    ldp x21, x22, [sp], #16
    ldp x23, x24, [sp], #16
    ldp x25, x26, [sp], #16
    ldp x27, x28, [sp], #16
    ldp x29, x30, [sp], #16

    ret
