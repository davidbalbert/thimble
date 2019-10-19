TOOLCHAIN := aarch64-elf

CFLAGS := -O0 -MD -fno-builtin -Wall -Werror -g -I. -Iarm64
ASFLAGS := -MD -g
LDFLAGS := -m aarch64elf -static -nostdlib -N

QEMU := qemu-system-aarch64

OBJS += \
	arm64/start.o\
	arm64/arch.o\
	arm64/cpu.o\
	arm64/uart.o\
	arm64/trap.o\
	arm64/ivec.o\
	arm64/lock.o\
	arm64/proc.o\

.PHONY: default
default: kernel

.PHONY: qemu
qemu: kernel
	$(QEMU) -M raspi3 -serial null -serial mon:stdio -kernel kernel -nographic

.PHONY: gdb
gdb: kernel
	$(QEMU) -M raspi3 -serial null -serial mon:stdio -kernel kernel -nographic -gdb tcp::1234 -S

-include arm64/*.d
