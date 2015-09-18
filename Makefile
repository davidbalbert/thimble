OBJS = \
       main.o\

TOOLCHAIN=x86_64-elf

CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
OBJCOPY = $(TOOLCHAIN)-objcopy

QEMU = qemu-system-x86_64

CFLAGS = -O0 -MD -m64 -fno-builtin -Wall -Werror
LDFLAGS = -m elf_x86_64 -static -nostdlib -N

kernel.img: mbr kernel
	dd bs=512 count=20 if=/dev/zero of=kernel.img
	dd bs=512 if=mbr of=kernel.img conv=notrunc
	dd bs=512 seek=1 if=kernel of=kernel.img conv=notrunc

kernel: $(OBJS) kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel $(OBJS)

mbr: boot1.S boot2.c boot.ld
	$(CC) $(CFLAGS) -c boot1.S
	$(CC) $(CFLAGS) -Os -c boot2.c
	$(LD) $(LDFLAGS) -N -T boot.ld -o mbr boot1.o boot2.o


-include *.d

.PHONY: clean
clean:
	rm -rf mbr kernel *.img *.o *.d

.PHONY: qemu
qemu: kernel.img
	qemu-system-x86_64 -monitor stdio -drive file=kernel.img,format=raw -m 512
