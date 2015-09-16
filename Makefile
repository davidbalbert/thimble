TOOLCHAIN=x86_64-elf

CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
OBJCOPY = $(TOOLCHAIN)-objcopy

QEMU = qemu-system-x86_64

CFLAGS = -O0 -MD
LDFLAGS = -m elf_x86_64

-include *.d

mbr: boot1.S boot2.c boot.ld
	$(CC) $(CFLAGS) -c boot1.S
	$(CC) $(CFLAGS) -c boot2.c
	$(LD) $(LDFLAGS) -N -T boot.ld -o mbr boot1.o boot2.o


.PHONY: clean
clean:
	rm -rf mbr *.o *.d

.PHONY: qemu
qemu: mbr
	qemu-system-x86_64 -monitor stdio -cpu SandyBridge mbr
