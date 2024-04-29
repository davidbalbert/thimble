ifneq ($(shell command -v x86_64-elf-gcc 2>/dev/null),)
	GNU_TOOLCHAIN := x86_64-elf
else
	GNU_TOOLCHAIN := x86_64-linux-gnu
endif

LD := $(GNU_TOOLCHAIN)-ld
LDFLAGS := -static -nostdlib --omagic

ifeq ($(TOOLCHAIN),Clang)
	CC := clang
	CFLAGS := -target x86_64-elf -O0 -MD -ffreestanding -Wall -Werror -mno-red-zone -mcmodel=large -g -I. -Ix86_64
	ASFLAGS := -target x86_64-elf -MD -g -I. -Ix86_64
else ifeq ($(TOOLCHAIN),GCC)
	CC := $(GNU_TOOLCHAIN)-gcc
	CFLAGS := -m64 -O0 -MD -ffreestanding -fno-pie -Wall -Werror -mno-red-zone -mcmodel=large -g -I. -Ix86_64
	ASFLAGS := -m64 -MD -g -I. -Ix86_64 -Wa,-divide
else
    $(error Unsupported TOOLCHAIN: $(TOOLCHAIN))
endif

QEMU := qemu-system-x86_64

QEMUOPTS := -monitor stdio -drive file=kernel.img,format=raw -m 512

Q35_QEMUOPTS := -monitor stdio \
	       -M q35 \
	       -m 512 \
	       -device ide-hd,drive=hd0,bus=ide.0 \
	       -drive file=kernel.img,format=raw,if=none,id=hd0 \

OBJS += \
	x86_64/start.o\
	x86_64/arch.o\
	x86_64/pic.o\
	x86_64/kbd.o\
	x86_64/swtch.o\
	x86_64/lock.o\
	x86_64/proc.o\
	x86_64/syscall.o\
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

LIBCOBJS += \
	   x86_64/libcasm.o\

kernel.img: x86_64/boot x86_64/stage2 x86_64/stage2size.txt kernel
	dd bs=512 count=16384 if=/dev/zero of=kernel.img
	dd bs=512 if=x86_64/boot of=kernel.img conv=notrunc
	dd bs=512 if=x86_64/stage2 of=kernel.img conv=notrunc seek=1
	dd bs=512 if=kernel of=kernel.img conv=notrunc seek=$(shell expr $(shell cat x86_64/stage2size.txt) + 1)

x86_64/boot.o: x86_64/stage2size.h

x86_64/boot: x86_64/boot.o x86_64/boot.ld
	$(LD) $(LDFLAGS) --script=x86_64/boot.ld --output=x86_64/boot x86_64/boot.o

x86_64/stage2size.txt: x86_64/stage2
	wc -c x86_64/stage2 | awk '{ print int(($$1 + 511) / 512) }' > x86_64/stage2size.txt

x86_64/stage2size.h: x86_64/stage2size.txt
	echo '#define STAGE2SIZE' $(shell cat x86_64/stage2size.txt) > x86_64/stage2size.h

STAGE2OBJS := \
	     x86_64/stage2asm.o\
	     x86_64/stage2.o\
	     x86_64/bootide.o\
	     x86_64/pci.o\
	     x86_64/ahci.o\
			 x86_64/vgacons.o\
			 x86_64/cpu.o\
			 console.o\
	     klibc.o\

x86_64/stage2: $(STAGE2OBJS) x86_64/stage2.ld
	$(LD) $(LDFLAGS) --script=x86_64/stage2.ld --output=x86_64/stage2 $(STAGE2OBJS)


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
