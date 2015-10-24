OBJS = \
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

TOOLCHAIN=x86_64-elf

CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
OBJCOPY = $(TOOLCHAIN)-objcopy

QEMU = qemu-system-x86_64

CFLAGS = -m64 -O0 -MD -fno-builtin -Wall -Werror -mcmodel=large -gdwarf-2
LDFLAGS = -m elf_x86_64 -static -nostdlib -N

kernel.img: stage1 stage2 kernel
	dd bs=512 count=256 if=/dev/zero of=kernel.img
	dd bs=512 if=stage1 of=kernel.img conv=notrunc
	dd bs=512 if=stage2 of=kernel.img conv=notrunc seek=1
	dd bs=512 if=kernel of=kernel.img conv=notrunc seek=2 # Assuming stage2 fits in one sector ¯\_(ツ)_/¯

kernel: $(OBJS) kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel $(OBJS)



stage1: boot1.S stage1.c bootide.c stage1.ld
	$(CC) $(CFLAGS) -c boot1.S
	$(CC) $(CFLAGS) -Os -c stage1.c
	$(CC) $(CFLAGS) -Os -c bootide.c
	$(LD) $(LDFLAGS) -N -T stage1.ld -o stage1 boot1.o stage1.o bootide.o

stage2: boot2.S stage2.c bootide.c stage2.ld
	$(CC) $(CFLAGS) -c boot2.S
	$(CC) $(CFLAGS) -Os -c stage2.c
	$(CC) $(CFLAGS) -Os -c bootide.c
	$(LD) $(LDFLAGS) -N -T stage2.ld -o stage2 boot2.o stage2.o bootide.o

ivec.S: ivec.rb
	ruby ivec.rb > ivec.S

-include *.d

.PHONY: clean
clean:
	rm -rf stage1 stage2 kernel ivec.S *.img *.o *.d


QEMUOPTS = -monitor stdio -drive file=kernel.img,format=raw -m 512

.PHONY: qemu
qemu: kernel.img
	$(QEMU) $(QEMUOPTS)

.PHONY: qemu-gdb
qemu-gdb: kernel.img
	@echo Run x86_64-elf-gdb
	$(QEMU) $(QEMUOPTS) -gdb tcp::12345

.PHONY: bochs
bochs: kernel.img
	bochs
