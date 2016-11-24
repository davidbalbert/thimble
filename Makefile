OBJS = \
       entry.o\
       main.o\
       console.o\
       trap.o\
       ivec.o\
       alltraps.o\
       pic.o\
       kbd.o\
       swtch.o\
       lock.o\
       proc.o\
       kalloc.o\
       klibc.o\
       timer.o\
       vm.o\
       syscall.o\
       syscallasm.o\
       file.o\

TOOLCHAIN=x86_64-elf

CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
OBJCOPY = $(TOOLCHAIN)-objcopy

QEMU = qemu-system-x86_64

CFLAGS = -m64 -O0 -MD -fno-builtin -Wall -Werror -mcmodel=large -g
ASFLAGS = -m64 -MD -g -Wa,-divide
LDFLAGS = -m elf_x86_64 -static -nostdlib -N

kernel.img: boot stage2 kernel stage2size.txt
	dd bs=512 count=16384 if=/dev/zero of=kernel.img
	dd bs=512 if=boot of=kernel.img conv=notrunc
	dd bs=512 if=stage2 of=kernel.img conv=notrunc seek=1
	dd bs=512 if=kernel of=kernel.img conv=notrunc seek=$(shell expr $(shell cat stage2size.txt) + 1)

kernel: $(OBJS) kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel $(OBJS)

main.c: task1.h

LIBCOBJS = \
	   klibc.o\
	   libc.o\
	   libcasm.o\

task1: task1.o $(LIBCOBJS)
	$(LD) $(LDFLAGS) -e main -Ttext=0 -o task1 $^

task1.h: task1
	xxd -i task1 > task1.h

boot.o: stage2size.h

boot: boot.o boot.ld
	$(LD) $(LDFLAGS) -N -T boot.ld -o boot boot.o

stage2size.txt: stage2
	wc -c stage2 | awk '{ print int(($$1 + 511) / 512) }' > stage2size.txt

stage2size.h: stage2size.txt
	echo '#define STAGE2SIZE' $(shell cat stage2size.txt) > stage2size.h


STAGE2OBJS = \
	     stage2asm.o\
	     stage2.o\
	     bootide.o\
	     console.o\
	     pci.o\
	     ahci.o\
	     klibc.o\

stage2: $(STAGE2OBJS) stage2.ld
	$(LD) $(LDFLAGS) -N -T stage2.ld -o stage2 $(STAGE2OBJS)

ivec.S: ivec.rb
	ruby ivec.rb > ivec.S

-include *.d

.PHONY: clean
clean:
	rm -rf boot stage2 kernel ivec.S stage2size.* *.img *.o *.d task1 task1.h


QEMUOPTS = -monitor stdio -drive file=kernel.img,format=raw -m 512

Q35_QEMUOPTS = -monitor stdio \
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
