.globl alltraps
alltraps:
    mov     %rsp, %rax
    call    trap
    add     $16, %rsp  # trapno and error
    iretq
  
