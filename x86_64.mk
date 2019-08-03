TOOLCHAIN := x86_64-elf

QEMU := qemu-system-x86_64

CFLAGS := -m64 -O0 -MD -fno-builtin -Wall -Werror -mcmodel=large -g
ASFLAGS := -m64 -MD -g -Wa,-divide
LDFLAGS := -m elf_x86_64 -static -nostdlib -N

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



.PHONY: default
default: kernel.img

QEMUOPTS := -monitor stdio -drive file=kernel.img,format=raw -m 512

Q35_QEMUOPTS := -monitor stdio \
	       -M q35 \
	       -m 512 \
	       -device ide-hd,drive=hd0,bus=ide.0 \
	       -drive file=kernel.img,format=raw,if=none,id=hd0 \

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
