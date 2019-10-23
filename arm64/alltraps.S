.global alltraps
alltraps:
    stp x29, x28, [sp, #-16]!
    stp x27, x26, [sp, #-16]!
    stp x25, x24, [sp, #-16]!
    stp x23, x22, [sp, #-16]!
    stp x21, x20, [sp, #-16]!
    stp x19, x18, [sp, #-16]!
    stp x17, x16, [sp, #-16]!
    stp x15, x14, [sp, #-16]!
    stp x13, x12, [sp, #-16]!
    stp x11, x10, [sp, #-16]!
    stp x9, x8, [sp, #-16]!
    stp x7, x6, [sp, #-16]!
    stp x5, x4, [sp, #-16]!
    stp x3, x2, [sp, #-16]!
    stp x1, x0, [sp, #-16]!

    mrs x1, esr_el1
    mrs x2, elr_el1
    stp x1, x2, [sp, #-16]!

    mrs x1, spsr_el1
    mrs x2, far_el1
    stp x1, x2, [sp, #-16]!

    mov sp, x0 // put a pointer to the TrapFrame on top of the stack

    b trap

    // TODO: pop everything off the stack. Remember x30 and the exception number
    // which are in ivec.S

    eret
