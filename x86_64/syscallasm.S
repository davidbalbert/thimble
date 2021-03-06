#include "mem.h"
#include "syscall.h"

#define EFER  0xC0000080    // extended feature enable register
#define STAR  0xC0000081    // kernel mode %ss and %cs
#define LSTAR 0xC0000082    // %rip for syscall entry
#define FMASK 0xC0000084

#define SCE   0             // syscall enable bit of EFER


# Note: This has to mirror the stack structure in
# mkproc. If you make changes here, you must make
# the same changes there!

.global syscall0
syscall0:
    cli                 # make sure we're not interrupted in kernel mode on the user stack

    push    %r12        # temporary storage
    push    %r13

    push    %rbp
    push    %r11        # userspace flags
    push    %rcx        # return address

    mov     %rax, %r12  # temporarily save syscall num

    # save syscall args before calling kstacktop
    push    %r9
    push    %r8
    push    %r10
    push    %rdx
    push    %rsi
    push    %rdi

    call    kstacktop   # top of kstack -> rax

    pop     %rdi
    pop     %rsi
    pop     %rdx
    pop     %r10
    pop     %r8
    pop     %r9


    mov     %rsp, %r13  # temp user stack pointer

    #####################

    mov     %rax, %rsp  # switch to the kernel stack

    sti                 # we're on the kernel stack, so we can get interrupted again.

    # build a SyscallFrame
    push    %r9
    push    %r8
    push    %r10
    push    %rdx
    push    %rsi
    push    %rdi
    push    %r12        # syscall number
    push    %r13        # user rsp
    push    $0          # syscall return value

    mov     %rsp, %rdi  # SyscallFrame as first arg to syscall

    call    syscall

.global sysret
sysret:
    # get rid of SyscallFrame
    pop     %rax
    pop     %r13
    pop     %r12
    pop     %rdi
    pop     %rsi
    pop     %rdx
    pop     %r10
    pop     %r8
    pop     %r9

    #####################

    cli                 # make sure we don't get interrupted while in kernel
                        # mode on the user stack. Sysret will renable interrupts
                        # when %rflags is restored from %r11. See SYSRET in the
                        # Intel Software Developer's Manual.
                        #
                        # TODO: disable NMIs as well.

    mov     %r13, %rsp  # switch back to the user stack

    pop     %rcx        # restore return address
    pop     %r11        # restore flags
    pop     %rbp

    pop     %r13        # temporary storage
    pop     %r12

    sysretq

.global sysinit
sysinit:
    # Enable syscall/sysret
    # There's a lot of poking at MSRs. Rdmsr and wrmsr operate on
    # three registers: the MSR address is stored in ecx and the
    # 64-bit MSR value is stored in edx:eax.

    # Set the SCE bit of EFER
    mov     $EFER, %ecx
    rdmsr
    or      $(1 << SCE), %eax
    wrmsr

    # Put syscall entry point in LSTAR
    mov     $LSTAR, %ecx
    movabs  $syscall0, %rax
    mov     %rax, %rdx
    shr     $32, %rdx
    wrmsr

    # Put segment selectors in STAR. Bytes 6 and 7 (top two bytes of edx) is
    # SEG_UCODE32 for sysret. Bytes 4 and 5 are SEG_KCODE for syscall. Syscall
    # and sysret assume that the data segments for the stack segment selector
    # come directly after the code segments.
    mov     $STAR, %ecx
    rdmsr
    xor     %edx, %edx
    mov     $((SEG_UCODE32 << 3 | USER_DPL) << 16), %edx
    or      $(SEG_KCODE << 3), %edx
    wrmsr

    ret
