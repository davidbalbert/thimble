TOOLCHAIN := x86_64-elf

CFLAGS := -m64 -O0 -MD -fno-builtin -Wall -Werror -mcmodel=large -g -I. -Ix86_64
ASFLAGS := -m64 -MD -g -I. -Ix86_64 -Wa,-divide
LDFLAGS := -static -nostdlib -N

QEMU := qemu-system-x86_64

QEMUOPTS := -monitor stdio -drive file=kernel.img,format=raw -m 512

Q35_QEMUOPTS := -monitor stdio \
	       -M q35 \
	       -m 512 \
	       -device ide-hd,drive=hd0,bus=ide.0 \
	       -drive file=kernel.img,format=raw,if=none,id=hd0 \

OBJS += \
	entry.o\
	console.o\
	trap.o\
	ivec.o\
	alltraps.o\
	pic.o\
	kbd.o\
	swtch.o\
	lock.o\
	proc.o\
	klibc.o\
	timer.o\
	vm.o\
	syscallasm.o\
	x86_64/vgacons.o\


.PHONY: default
default: kernel.img

kernel.img: x86_64/boot x86_64/stage2 x86_64/stage2size.txt kernel
	dd bs=512 count=16384 if=/dev/zero of=kernel.img
	dd bs=512 if=boot of=kernel.img conv=notrunc
	dd bs=512 if=stage2 of=kernel.img conv=notrunc seek=1
	dd bs=512 if=kernel of=kernel.img conv=notrunc seek=$(shell expr $(shell cat stage2size.txt) + 1)


.PHONY: qemu
qemu: kernel.img
	$(QEMU) $(Q35_QEMUOPTS)

.PHONY: qemu-gdb
qemu-gdb: kernel.img
	@echo Run x86_64-elf-gdb
	$(QEMU) $(Q35_QEMUOPTS) -gdb tcp::1234 -S

.PHONY: bochs
bochs: kernel.img
	bochs
