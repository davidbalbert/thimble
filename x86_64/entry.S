# Entry point to our kernel. Here we switch to a statically
# allocated kernel stack and then jump to main.

#include "mem.h"

.code64

.global _start
_start:
    movabs $(kstack + KSTACKSIZE), %rsp
    jmp main

.comm kstack, KSTACKSIZE
