# Temporary hack to make sure that our binary user programs always start at main

.global _start
_start:
    jmp main
