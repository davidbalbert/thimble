OBJS = \
       main.o\

TOOLCHAIN=x86_64-elf

CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
OBJCOPY = $(TOOLCHAIN)-objcopy

QEMU = qemu-system-x86_64

CFLAGS = -m64 -O0 -MD -fno-builtin -Wall -Werror
LDFLAGS = -m elf_x86_64 -static -nostdlib -N

kernel.img: boot stage2 kernel
	dd bs=512 count=20 if=/dev/zero of=kernel.img
	dd bs=512 if=boot of=kernel.img conv=notrunc
	dd bs=512 if=stage2 of=kernel.img conv=notrunc seek=1
	dd bs=512 if=kernel of=kernel.img conv=notrunc seek=2 # Assuming stage2 fits in one sector ¯\_(ツ)_/¯

kernel: $(OBJS) kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel $(OBJS)

boot: boot.S boot1.c boot.ld
	$(CC) $(CFLAGS) -c boot.S
	$(CC) $(CFLAGS) -Os -c boot1.c
	$(LD) $(LDFLAGS) -N -T boot.ld -o boot boot.o boot1.o

stage2: boot2.S stage2.c
	$(CC) $(CFLAGS) -c boot2.S
	$(CC) $(CFLAGS) -Os -c stage2.c
	$(LD) $(LDFLAGS) -N -T stage2.ld -o stage2 boot2.o stage2.o

-include *.d

.PHONY: clean
clean:
	rm -rf boot kernel *.img *.o *.d

.PHONY: qemu
qemu: kernel.img
	qemu-system-x86_64 -monitor stdio -drive file=kernel.img,format=raw -m 512
