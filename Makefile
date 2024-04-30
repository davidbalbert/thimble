ARCH ?= x86_64
TOOLCHAIN ?= GCC

OBJS := \
		main.o\
		syscall.o\
		vm.o\
		console.o\
		klibc.o\
		kalloc.o\
		file.o\
		proc.o\

LIBCOBJS := \
	   klibc.o\
	   libc.o\

include $(ARCH)/$(ARCH).mk

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
	rm -rf x86_64/boot x86_64/stage2 kernel x86_64/ivec.S x86_64/stage2size.* *.img *.o *.d x86_64/*.o x86_64/*.d arm64/*.o arm64/*.d task1 task1.h
