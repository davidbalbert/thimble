TOOLCHAIN := aarch64-elf

CFLAGS := -O0 -MD -fno-builtin -Wall -Werror -g
ASFLAGS := -MD -g
LDFLAGS := -m aarch64elf -static -nostdlib -N

QEMU := qemu-system-aarch64

OBJS += \
	arm64/start.o\
	arm64/cpu.o\

.PHONY: default
default: kernel

.PHONY: qemu
qemu: kernel
	$(QEMU) -M raspi3 -serial null -serial mon:stdio -kernel kernel
