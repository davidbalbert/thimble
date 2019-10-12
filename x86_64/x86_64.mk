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
	x86_64/start.o\
	x86_64/pic.o\
	x86_64/kbd.o\
	x86_64/swtch.o\
	x86_64/lock.o\
	proc.o\
	kalloc.o\
	file.o\
	syscall.o\
	x86_64/timer.o\
	x86_64/vm.o\
	x86_64/syscallasm.o\
	x86_64/vgacons.o\
	x86_64/cpu.o\
	x86_64/trap.o\
	x86_64/ivec.o\
	x86_64/alltraps.o\

.PHONY: default
default: kernel.img

x86_64/ivec.S: x86_64/ivec.rb
	ruby x86_64/ivec.rb > x86_64/ivec.S

main.c: task1.h

LIBCOBJS := \
	   klibc.o\
	   libc.o\
	   libcasm.o\

task1: task1.o $(LIBCOBJS)
	$(LD) $(LDFLAGS) -e main -Ttext=0 -o task1 $^

task1.h: task1
	xxd -i task1 > task1.h

kernel.img: x86_64/boot x86_64/stage2 x86_64/stage2size.txt kernel
	dd bs=512 count=16384 if=/dev/zero of=kernel.img
	dd bs=512 if=x86_64/boot of=kernel.img conv=notrunc
	dd bs=512 if=x86_64/stage2 of=kernel.img conv=notrunc seek=1
	dd bs=512 if=kernel of=kernel.img conv=notrunc seek=$(shell expr $(shell cat x86_64/stage2size.txt) + 1)


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
