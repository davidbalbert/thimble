ifneq ($(shell command -v x86_64-elf-gcc 2>/dev/null),)
	GNU_TOOLCHAIN := x86_64-elf
else
	GNU_TOOLCHAIN := x86_64-linux-gnu
endif

LD := $(GNU_TOOLCHAIN)-ld
LDFLAGS := -m aarch64elf -static -nostdlib -N

ifeq ($(TOOLCHAIN),Clang)
	CC := clang
	CFLAGS := -target aarch64-elf -O0 -MD -ffreestanding -Wall -Werror -g -I. -Iarm64
	ASFLAGS := -target aarch64-elf -MD -g
else ifeq ($(TOOLCHAIN),GCC)
	CC := $(GNU_TOOLCHAIN)-gcc
	CFLAGS := -O0 -MD -ffreestanding -Wall -Werror -g -I. -Iarm64
	ASFLAGS := -MD -g
else
    $(error Unsupported TOOLCHAIN: $(TOOLCHAIN))
endif

QEMU := qemu-system-aarch64

OBJS += \
	arm64/start.o\
	arm64/arch.o\
	arm64/cpu.o\
	arm64/uart.o\
	arm64/trap.o\
	arm64/alltraps.o\
	arm64/ivec.o\
	arm64/lock.o\
	arm64/proc.o\
	arm64/vm.o\
	arm64/vmdbg.o\
	arm64/timer.o\
	arm64/bcmint.o\
	arm64/syscall.o\
	arm64/swtch.o\

.PHONY: default
default: kernel

LIBCOBJS += \
	   arm64/libcasm.o\

.PHONY: qemu
qemu: kernel
	$(QEMU) -M raspi3b -serial null -serial mon:stdio -kernel kernel -nographic

.PHONY: gdb
gdb: kernel
	$(QEMU) -M raspi3b -serial null -serial mon:stdio -kernel kernel -nographic -gdb tcp::1234 -S

-include arm64/*.d
