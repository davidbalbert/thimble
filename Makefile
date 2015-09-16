TOOLCHAIN=x86_64-elf

CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
OBJCOPY = $(TOOLCHAIN)-objcopy

QEMU = qemu-system-x86_64

CFLAGS = -O0 -MD
LDFLAGS = -m elf_x86_64

kernel.img: mbr hello.txt
	dd bs=512 count=20 if=/dev/zero of=kernel.img
	dd bs=512 if=mbr of=kernel.img conv=notrunc
	dd bs=512 seek=1 if=hello.txt of=kernel.img conv=notrunc

hello.txt:
	echo "Hello, world" > hello.txt

mbr: boot1.S boot2.c boot.ld
	$(CC) $(CFLAGS) -c boot1.S
	$(CC) $(CFLAGS) -Os -c boot2.c
	$(LD) $(LDFLAGS) -N -T boot.ld -o mbr boot1.o boot2.o


-include *.d

.PHONY: clean
clean:
	rm -rf mbr kernel hello.txt *.img *.o *.d

.PHONY: qemu
qemu: mbr
	qemu-system-x86_64 -monitor stdio -cpu SandyBridge -drive file=kernel.img,format=raw
