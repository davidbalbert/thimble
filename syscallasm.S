#include "mem.h"

#define EFER  0xC0000080    // extended feature enable register
#define STAR  0xC0000081    // kernel mode %ss and %cs
#define LSTAR 0xC0000082    // %rip for syscall entry
#define FMASK 0xC0000084

#define SCE   0                // syscall enable bit of EFER

.globl syscall0
syscall0:
    push    %r11        # userspace flags
    push    %rcx        # return address

    mov     %rax, %rdi  # save syscall num as first arg to syscall

    call    kstacktop   # top of kstack -> rax

    mov     %rsp, %rdx  # temp user stack pointer
    mov     %rax, %rsp  # switch to the kernel stack
    push    %rdx        # save the user stack pointer on kernel stack

    call    syscall

    pop     %rsp        # switch back to the user stack
    pop     %rcx        # restore return address
    pop     %r11        # restore flags
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

    # Put segment selectors in STAR. Bytes 6 and 7 (top two bytes
    # of edx) is SEG_UCODE for sysret. Bytes 4 and 5 are SEG_KCODE
    # for syscall. Syscall and sysret assume that the data segments
    # for the stack segment selector come directly after the code
    # segments.
    mov     $STAR, %ecx
    rdmsr
    xor     %edx, %edx
    mov     $(SEG_UCODE << (16 + 3)), %edx  # 16 to shift two bytes, 3 to make the selector
    or      $(SEG_KCODE << 3), %edx
    wrmsr

    ret
