
.align 11
.global vectors
vectors:
// TODO: we probably don't want the same 4 vectors for all 4 classes of exceptions
.rept 4
    // synchronous
    .align  7
    mov     x0, #0
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       trap

    // IRQ
    .align  7
    mov     x0, #1
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       trap

    // FIQ
    .align  7
    mov     x0, #2
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       trap

    // SError
    .align  7
    mov     x0, #3
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    b       trap
.endr
