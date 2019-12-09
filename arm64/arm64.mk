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
	arm64/sd.o\
	arm64/dma.o\

LIBCOBJS += \
	arm64/libcasm.o\

CLEAN += \
	kernel8.img\
	sd.img\
	arm64/*.o\
	arm64/*.d\

.PHONY: default
default: kernel8.img

kernel8.img: kernel
	cp kernel kernel8.img

sd.img: kernel8.img mkfs.sh
	./mkfs.sh sd.img

.PHONY: qemu
qemu: kernel8.img sd.img
	$(QEMU) -M raspi3 -serial null -serial mon:stdio -kernel kernel8.img -drive file=sd.img,if=sd,format=raw -nographic

.PHONY: gdb
gdb: kernel8.img
	$(QEMU) -M raspi3 -serial null -serial mon:stdio -kernel kernel8.img -drive file=sd.img,if=sd,format=raw -nographic -gdb tcp::1234 -S

-include arm64/*.d
