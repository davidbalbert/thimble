# void swtch(Registers **from, Registers *to)
#   %rdi - from
#   %rsi - to
#
# Registers are stored on the top of the stack. From is a double
# pointer so that we can store the address of the top of the stack
# once we have pushed everything.
#
# The structure of swtch mirrors that of Registers.
#
# The general shape of this function is as follows
#   - Save the registers for the current process on its stack (which is the current one).
#   - Save the address of the Registers into `from`. This doubles as our stack pointer.
#   - Switch to the new process's stack (the stack pointer is the same as `to`).
#   - Restore the new process's registers from its stack.
#
# Swtch must be called while holding ptable.lock. The caller must release
# ptable.lock after swtch returns, unless the caller is the scheduler.
.globl swtch
swtch:
    # First, store the current callee saved registers onto
    # the stack. This will be `from`, which we're building up.
    # %rip has already been pushed onto the stack when swtch
    # was called.
    push    %r15
    push    %r14
    push    %r13
    push    %r12
    push    %rbx
    push    %rbp

    # %rsp now points to the old saved Registers. Store it in *from.
    mov     %rsp, (%rdi)

    # Our new stack pointer is the same as to.
    mov     %rsi, %rsp

    pop     %rbp
    pop     %rbx
    pop     %r12
    pop     %r13
    pop     %r14
    pop     %r15

    ret
