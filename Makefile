OBJS := \
	main.o\
	syscall.o\
	vm.o\
	console.o\
	klibc.o\
	kalloc.o\
	file.o\
	proc.o\
	bio.o\
	sleeplock.o\
	xxd.o\
	fat.o\

LIBCOBJS := \
	klibc.o\
	libc.o\

CLEAN := \
	kernel\
	*.o\
	*.d\
	task1\
	task1.h\

include $(ARCH)/$(ARCH).mk

CC := $(TOOLCHAIN)-gcc
LD := $(TOOLCHAIN)-ld
OBJCOPY := $(TOOLCHAIN)-objcopy

kernel: $(OBJS) $(ARCH)/kernel.ld
	$(LD) $(LDFLAGS) -T $(ARCH)/kernel.ld -o kernel $(OBJS)

main.c: task1.h

task1: task1.o $(LIBCOBJS)
	$(LD) $(LDFLAGS) -e main -Ttext=0 -o task1 $^

task1.h: task1
	xxd -i task1 > task1.h

-include *.d

.PHONY: clean
clean:
	rm -rf $(CLEAN)
