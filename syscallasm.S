#include "mem.h"
#include "syscall.h"

#define EFER  0xC0000080    // extended feature enable register
#define STAR  0xC0000081    // kernel mode %ss and %cs
#define LSTAR 0xC0000082    // %rip for syscall entry
#define FMASK 0xC0000084

#define SCE   0                // syscall enable bit of EFER

.globl syscall0
syscall0:
    push    %r12        # temporary storage
    push    %r13

    push    %r11        # userspace flags
    push    %rcx        # return address

    mov     %rax, %r12  # temporarily save syscall num

    call    kstacktop   # top of kstack -> rax

    mov     %rsp, %r13  # temp user stack pointer
    mov     %rax, %rsp  # switch to the kernel stack
    push    %r13        # save the user stack pointer on kernel stack

    #push    %r9
    #push    %r8
    #push    %r10
    #push    %rdx
    #push    %rsi
    #push    %rdi
    #push    %r12

    mov     %r12, %rdi  # syscall num as first arg to syscall
    call    syscall

.globl sysret
sysret:
    #pop     %r12
    #pop     %rdi
    #pop     %rsi
    #pop     %rdx
    #pop     %r10
    #pop     %r8
    #pop     %r9

    pop     %rsp        # switch back to the user stack

    pop     %rcx        # restore return address
    pop     %r11        # restore flags

    pop     %r13        # temporary storage
    pop     %r12

    sysretq

.globl sysinit
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

.globl hello
hello:
    mov     $SYS_HELLO, %rax

    # x86_64 System V user calling convention uses %rcx for the 4th function
    # agument, but the syscall interface uses %r10 (%rcx is used by the syscall
    # instruction to store the userspace %rip). We make the translation here.
    #
    # Note: The kernel uses the user calling convention internally. It's just
    # the syscall interface (userspace to kernelspace) that requires this
    # translation.
    mov     %rcx, %r10
    syscall
    ret

.globl goodbye
goodbye:
    mov     $SYS_GOODBYE, %rax

    # x86_64 System V user calling convention uses %rcx for the 4th function
    # agument, but the syscall interface uses %r10 (%rcx is used by the syscall
    # instruction to store the userspace %rip). We make the translation here.
    #
    # Note: The kernel uses the user calling convention internally. It's just
    # the syscall interface (userspace to kernelspace) that requires this
    # translation.
    mov     %rcx, %r10
    syscall
    ret
