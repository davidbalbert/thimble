# Thimble - A small OS that doesn't do much

## Current status

Supports PC with legacy BIOS (x86_64) and Raspberry Pi 3B (aarch64). Doesn't boot on a physical Raspberry Pi. Untested on a physical PC.

On boot, initializes hardware, scheduler, virtual memory, and then spawns the first user space process ([task1.c](/task1.c)) which forks, and then parent and child start counting up to infinity.

## Features

- Multiplatform.
- Preemptive multitasking.
- Protected memory.
- User space processes.
- Custom bootloader (PC only).
- Drivers: IDE, AHCI, PCI, VGA (text mode), keyboard, PIC, timers, UART.

## Points of interest

- PC bootloader ([stage 1](/x86_64/boot.S), [stage 2](/x86_64/stage2.c)).
- Console ([generic interface](/console.c), [VGA console (PC)](/x86_64/vgacons.c), [UART console (raspi)](/arm64/uart.c)).
- [Scheduler](/proc.c) and [context switching](/x86_64/swtch.S).
- [Page allocator](/kalloc.c) and virtual memory system ([vm.c](/vm.c), [x86_64/vm.c](/x86_64/vm.c), [arm64/vm.c](/arm64/vm.c))

## Requirements

Thimble should compile on Linux or macOS.

- Bash
- A toolchain (see below)
- Make
- Ruby (any version should do)
- xxd
- QEMU (qemu-system-x86_64, qemu-system-aarch64)

## Toolchains

Thimble can be built with GCC and Clang. Regardless of what compiler you're using, Thimble uses GNU ld because lld can't currently run our linker scripts.

For GCC and GNU Binutils, the following targets are supported:
- x86_64: x86_64-elf and x86_64-linux-gnu
- arm64: aarch64-elf and aarch64-linux-gnu

### Using system toolchains

#### macOS

```
$ brew install aarch64-elf-binutils
$ brew install x86_64-elf-binutils
```

For Clang, use the version shipped with Apple's Command Line Tools for Xcode.

For GCC:

```
$ brew install aarch64-elf-gcc
$ brew install x86_64-elf-gcc
```

#### Linux

Install the appropriate cross compilers for your distro. On Debian:

```
$ sudo apt install build-essential
$ sudo apt install binutils-x86-64-linux-gnu
$ sudo apt install binutils-aarch64-linux-gnu
```

For Clang:

```
$ sudo apt install clang
```

For GCC:

```
$ sudo apt install gcc-x86-64-linux-gnu
$ sudo apt install gcc-aarch64-linux-gnu
```

### Building toolchains from source

Thimble ships with mktoolchain.bash, which can build the necessary toolchains from source. Packages will be installed in the `toolchain` directory.

You can pass either or both of the `--gcc` or `--clang` options to choose which compiler(s) to build. If you don't specify a compiler, GCC will be built by default. Mktoolchain.bash is idempotent and won't do unnecessary work.

Building Clang requires CMake >= 3.20.0. If you don't have a recent enough version of CMake installed, mktoolchain.bash will build it for you.

Tarballs will be downloaded and extracted into `toolchain/tmp`. By default, they will be deleted after the script finishes running. To prevent this, pass `--skip-cleanup`.

To build the both GCC and Clang:

```
$ ./mktoolchain.bash --gcc --clang
```

#### macOS

Apple's Command Line Tools for Xcode is enough to run mktoolchain.bash. To speed things up, install CMake using Homebrew first.

**NOTE:** GCC 13 doesn't build with the Xcode 15 developer tools ([see more here](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111632)), so mktoolchain.bash will refuse to build GCC on macOS. This should be fixed in GCC 14.

#### Linux

Required packages:

- The system C compiler
- curl
- m4 (GCC)
- CMake (Clang)

If your distro doesn't package a recent enough version of CMake, you must also install libssl-dev (or equivalent).

## Building and running

Thimble is built with Make. You use the `ARCH` and `TOOLCHAIN` variables to determine what to build and how to build it.

`ARCH` can be either "x86_64" or "arm64". `TOOLCHAIN` can be either "GCC" or "Clang". By default, Thimble will build for x86_64 with GCC.

Some examples:

```
# Build `kernel.img`, a disk image for PC (with an MBR, bootloader, etc.), using GCC and run it in QEMU.
$ make qemu

# Do the same, but using Clang.
$ make qemu TOOLCHAIN=Clang

# Build `kernel`, an ELF executable, using GCC and run it on an emulated Raspberry Pi 3B in QEMU. The
# kernel is loaded by QEMU directly.
$ make qemu ARCH=arm64

# Do the same using Clang.
$ make qemu ARCH=arm64 TOOLCHAIN=Clang

# Delete all artifacts and generated files.
$ make clean
```

If you built a toolchain from source, you need to add `$PATH_TO_THIMBLE_SOURCE/toolchain` to $PATH. An easy way to do this is to source `env.bash`. This will also do some other niceties including setting up your $MANPATH correctly, and configuring Make to run in parallel based on the number of CPU cores on your computer. Much like a Python environment, you can deactivate when finished and your shell will be restored to its previous state.

```
$ source env.bash
(thimble) $ make qemu
(thimble) $ deactivate
$
```

## Xv6

Thimble takes a lot of inspiration (including coding conventions and function naming) and some source code from [xv6](https://pdos.csail.mit.edu/6.828/2016/xv6.html). As Thimble has grown, it has become more distinct from xv6, but there are a lot of similar design decisions that should feel familiar to anyone who knows the xv6 codebase.

For the xv6 license, see `ACKNOWLEDGEMENTS`

# License

Thimble is copyright David Albert and released under the terms of the MIT License. See LICENSE.txt for details.

