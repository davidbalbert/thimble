# This is the first file passed to the linker for stage2. It exists to ensure
# that there is known code at the beginning of stage2.

.code64

start:
    jmp stage2main
