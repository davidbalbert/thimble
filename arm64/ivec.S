
.align 11
.global vectors
vectors:
// TODO: we probably don't want the same 4 vectors for all 4 classes of exceptions
.rept 4
    // synchronous
    .align  7
    // We push x30 directly in the vector to keep stack alignment at 16 bytes.
    // The rest of the registers are pushed in alltraps.
    stp #0, x30, [sp, #-16]!
    b alltraps

    // IRQ
    .align  7
    stp #1, x30, [sp, #-16]!
    b alltraps

    // FIQ
    .align  7
    stp #2, x30, [sp, #-16]!
    b alltraps

    // SError
    .align  7
    stp #3, x30, [sp, #-16]!
    b alltraps
.endr
