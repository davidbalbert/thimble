TOOLCHAIN := aarch64-elf

QEMU := qemu-system-aarch64

CFLAGS := -O0 -MD -fno-builtin -Wall -Werror -g
ASFLAGS := -MD -g
LDFLAGS := -m aarch64elf -static -nostdlib -N

.PHONY: default
default: kernel

qemu: kernel
	@echo Not implemented
