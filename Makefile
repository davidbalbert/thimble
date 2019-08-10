OBJS := \
       main.o\
       #kalloc.o\
       #syscall.o\
       #file.o\

include $(ARCH).mk

CC := $(TOOLCHAIN)-gcc
LD := $(TOOLCHAIN)-ld
OBJCOPY := $(TOOLCHAIN)-objcopy

kernel.img: boot stage2 kernel stage2size.txt
	dd bs=512 count=16384 if=/dev/zero of=kernel.img
	dd bs=512 if=boot of=kernel.img conv=notrunc
	dd bs=512 if=stage2 of=kernel.img conv=notrunc seek=1
	dd bs=512 if=kernel of=kernel.img conv=notrunc seek=$(shell expr $(shell cat stage2size.txt) + 1)

kernel: $(OBJS) $(ARCH)/kernel.ld
	$(LD) $(LDFLAGS) -T $(ARCH)/kernel.ld -o kernel $(OBJS)

#main.c: task1.h

LIBCOBJS := \
	   klibc.o\
	   libc.o\
	   libcasm.o\

#task1: task1.o $(LIBCOBJS)
	#$(LD) $(LDFLAGS) -e main -Ttext=0 -o task1 $^

#task1.h: task1
	#xxd -i task1 > task1.h

boot.o: stage2size.h

boot: boot.o boot.ld
	$(LD) $(LDFLAGS) -N -T boot.ld -o boot boot.o

stage2size.txt: stage2
	wc -c stage2 | awk '{ print int(($$1 + 511) / 512) }' > stage2size.txt

stage2size.h: stage2size.txt
	echo '#define STAGE2SIZE' $(shell cat stage2size.txt) > stage2size.h


STAGE2OBJS := \
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
