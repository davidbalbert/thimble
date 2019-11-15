TOOLCHAIN := aarch64-elf

CFLAGS := -O0 -MD -ffreestanding -Wall -Werror -g -I. -Iarm64
ASFLAGS := -MD -g
LDFLAGS := -m aarch64elf -static -nostdlib -N

QEMU := qemu-system-aarch64

OBJS += \
	arm64/start.o\
	arm64/arch.o\
	arm64/cpu.o\
	arm64/uart.o\
	arm64/trap.o\
	arm64/alltraps.o\
	arm64/ivec.o\
	arm64/lock.o\
	arm64/proc.o\
	arm64/vm.o\
	arm64/vmdbg.o\
	arm64/timer.o\
	arm64/bcmint.o\
	arm64/syscall.o\
	arm64/swtch.o\
	arm64/gpio.o\

LIBCOBJS += \
	   arm64/libcasm.o\

.PHONY: default
default: kernel

sd.img:
	./mkfs.sh sd.img

.PHONY: qemu
qemu: kernel
	$(QEMU) -M raspi3 -serial null -serial mon:stdio -kernel kernel -nographic

.PHONY: gdb
gdb: kernel
	$(QEMU) -M raspi3 -serial null -serial mon:stdio -kernel kernel -nographic -gdb tcp::1234 -S

-include arm64/*.d
