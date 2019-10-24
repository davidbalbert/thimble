
.align 11
.global vectors
vectors:
// TODO: we probably don't want the same 4 vectors for all 4 classes of exceptions
.rept 4
    // synchronous
    .align  7
    // We push x30 directly in the vector to keep stack alignment at 16 bytes.
    // The rest of the registers are pushed in alltraps. We have to do some
    // juggling to get the interrupt type, and x30 on the stack in the right
    // order without wiping out the content of x30 (or other registers).
    stp x30, xzr, [sp, #-16]!
    mov x30, #0 // now that x30 is saved, put the interrupt type on the stack
    str x30, [sp, #8]

    b alltraps

    // IRQ
    .align  7
    stp x30, xzr, [sp, #-16]!
    mov x30, #1
    str x30, [sp, #8]
    b alltraps

    // FIQ
    .align  7
    stp x30, xzr, [sp, #-16]!
    mov x30, #2
    str x30, [sp, #8]
    b alltraps

    // SError
    .align  7
    stp x30, xzr, [sp, #-16]!
    mov x30, #3
    str x30, [sp, #8]
    b alltraps
.endr
