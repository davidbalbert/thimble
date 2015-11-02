# void swtch(Registers **from, Registers *to)
#   %rdi - from
#   %rsi - to
#
# Registers are stored on the top of the stack. Swtch builds Registers for
# `from` on its stack, switches stacks, and then consumes `to`. `From` is a
# double pointer so that we can store the address of the new Registers struct
# we created.
#
# The order of pushes and pops depend on the layout of Registers.
#
# The general shape of this function is as follows
#   - Save the registers for the current process on its stack (this is the current stack).
#   - Save the address of our newly built Registers into `from`. `*from` doubles as our stack pointer.
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

.globl hello
    mov     $1, %rax
    syscall
    ret
