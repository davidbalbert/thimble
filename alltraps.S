.globl alltraps
alltraps:
    mov     %rsp, %rdi
    call    trap
    add     $16, %rsp  # trapno and error
    iretq
  
