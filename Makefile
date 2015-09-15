TOOLCHAIN=x86_64-elf

CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
OBJCOPY = $(TOOLCHAIN)-objcopy

QEMU = qemu-system-x86_64

CFLAGS = -O0 -MD
LDFLAGS = -m elf_x86_64

-include *.d

mbr: boot.S bootmain.c boot.ld makeboot.rb
	$(CC) $(CFLAGS) -c boot.S
	$(CC) $(CFLAGS) -c bootmain.c
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o mbr.o boot.o bootmain.o
	$(OBJCOPY) -S -O binary -j .text mbr.o mbr
	./makeboot.rb mbr


.PHONY: qemu
qemu: mbr
	qemu-system-x86_64 -monitor stdio -cpu SandyBridge mbr
