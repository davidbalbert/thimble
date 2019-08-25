OBJS := \
       main.o\
       console.o\
       klibc.o\
       #kalloc.o\
       #syscall.o\
       #file.o\

include $(ARCH)/$(ARCH).mk

CC := $(TOOLCHAIN)-gcc
LD := $(TOOLCHAIN)-ld
OBJCOPY := $(TOOLCHAIN)-objcopy

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

x86_64/boot.o: x86_64/stage2size.h

x86_64/boot: x86_64/boot.o x86_64/boot.ld
	$(LD) $(LDFLAGS) -N -T x86_64/boot.ld -o x86_64/boot x86_64/boot.o

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
	$(LD) $(LDFLAGS) -N -T x86_64/stage2.ld -o x86_64/stage2 $(STAGE2OBJS)

ivec.S: ivec.rb
	ruby ivec.rb > ivec.S

-include *.d

.PHONY: clean
clean:
	rm -rf x86_64/boot x86_64/stage2 kernel ivec.S x86_64/stage2size.* *.img *.o *.d $(ARCH)/*.o $(ARCH)/*.d task1 task1.h
