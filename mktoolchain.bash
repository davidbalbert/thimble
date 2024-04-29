#!/usr/bin/env bash

set -e

BINUTILS=https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.bz2

# GCC
GMP=https://ftp.gnu.org/gnu/gmp/gmp-6.3.0.tar.bz2
MPFR=https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.1.tar.bz2
MPC=https://ftp.gnu.org/gnu/mpc/mpc-1.3.1.tar.gz
GCC=https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz

# LLVM
CMAKE=https://github.com/Kitware/CMake/releases/download/v3.29.2/cmake-3.29.2.tar.gz
LLVM=https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.4/llvm-project-18.1.4.src.tar.xz

source "$(dirname "$0")/env.bash"

function get() {
  local name

  rm -rf "$(dir "$1")"
  name="$(basename "$1")"

  if [ ! -f "$name" ]; then
    echo "Downloading $name..."
    curl --progress-bar --location --output "$name" "$1"
  fi

  echo Extracting...
  tar xf "$name"
}

function dir() {
  basename "$1" | sed 's/\.tar\..*$//'
}

GCC=0
CLANG=0

SKIP_CLEANUP=0

for arg in "$@"; do
  case $arg in
    --gcc)
      GCC=1
      ;;
    --clang)
      CLANG=1
      ;;
    --skip-cleanup)
      SKIP_CLEANUP=1
      ;;
    *)
      echo "Unknown argument: $arg"
      exit 1
      ;;
  esac
done

if [ "$GCC" -eq 0 ] && [ "$CLANG" -eq 0 ]; then
  CLANG=1
fi

if [ "$GCC" -eq 1 ] && [ "$(uname)" = "Darwin" ]; then
  echo "Error: GCC 13 doesn't build with Xcode 15. This should be fixed in GCC 14."
  echo "See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111632 for more info."
  exit 1
fi

mkdir -p "$PREFIX/tmp"

cd "$PREFIX/tmp"

# We always install binutils because lld has the following issues with some of our linker
# scripts. Specifically:
#
# - It doesn't support adding the boot signature in boot.ld.
# - It doesn't support discarding .shstrtab even when output format is binary.

# It would be nicer to extract binutils once and build it out of tree in build-x86_64
# and build-aarch64 directories, but building binutils out of tree seems to require
# textinfo, which we'd like to avoid.
if [ ! -x "$PREFIX/bin/x86_64-elf-ld" ]; then
  get "$BINUTILS"
  cd "$(dir $BINUTILS)"
  ./configure --prefix="$PREFIX" --target=x86_64-elf --disable-werror
  make
  make install
  cd ..
fi

if [ ! -x "$PREFIX/bin/aarch64-elf-ld" ]; then
  # Won't re-download but will re-extract.
  get "$BINUTILS"
  cd "$(dir $BINUTILS)"
  ./configure --prefix="$PREFIX" --target=aarch64-elf --disable-werror
  make
  make install
  cd ..
fi

if [ "$CLANG" -eq 1 ]; then
  if ! type cmake >/dev/null 2>&1 || [ "$(cmake -P $llvm_cmake_version.cmake)" -ne 1 ]; then
    get "$CMAKE"
    cd "$(dir $CMAKE)"
    ./bootstrap --prefix="$PREFIX" --parallel="$NCORES"
    make
    make install
    cd ..
  fi

  if [[ ! -x $PREFIX/bin/clang ]]; then
    get "$LLVM"
    mkdir -p "$(dir $LLVM)/build"
    cd "$(dir $LLVM)/build"
    cmake -DCMAKE_INSTALL_PREFIX="$PREFIX" -DLLVM_ENABLE_PROJECTS="clang;lldb" -DCMAKE_BUILD_TYPE=Release -DLLDB_INCLUDE_TESTS=OFF -G "Unix Makefiles" ../llvm
    make
    make install
    cd ../..
  fi
fi

if [ "$GCC" -eq 1 ]; then
  if [ ! -f "$PREFIX/lib/libgmp.a" ]; then
    get "$GMP"
    cd "$(dir $GMP)"
    ./configure --prefix="$PREFIX"
    make
    make install
    cd ..
  fi

  if [ ! -f "$PREFIX/lib/libmpfr.a" ]; then
    get "$MPFR"
    cd "$(dir $MPFR)"
    ./configure --prefix="$PREFIX" --with-gmp="$PREFIX"
    make
    make install
    cd ..
  fi

  if [ ! -f "$PREFIX/lib/libmpc.a" ]; then
    get "$MPC"
    cd "$(dir $MPC)"
    ./configure --prefix="$PREFIX" --with-gmp="$PREFIX"
    make
    make install
    cd ..
  fi

  if [ ! -x "$PREFIX/bin/x86_64-elf-gcc" ] || [ ! -x "$PREFIX/bin/aarch64-elf-gcc" ]; then
    get "$GCC"
    cd "$(dir $GCC)"

    if [ ! -x "$PREFIX/bin/x86_64-elf-gcc" ]; then
      mkdir build-x86_64
      cd build-x86_64
      ../configure --prefix="$PREFIX" --target=x86_64-elf --disable-werror \
        --disable-libssp --disable-libmudflap --with-newlib --without-headers \
        --enable-languages=c,c++,objc,lto --enable-lto --with-gmp="$PREFIX"
      make all-gcc
      make install-gcc
      make all-target-libgcc
      make install-target-libgcc
      cd ..
    fi

    if [ ! -x "$PREFIX/bin/aarch64-elf-gcc" ]; then
      mkdir build-aarch64
      cd build-aarch64
      ../configure --prefix="$PREFIX" --target=aarch64-elf --disable-werror \
        --disable-libssp --disable-libmudflap --with-newlib --without-headers \
        --enable-languages=c,c++,objc,lto --enable-lto --with-gmp="$PREFIX"
      make all-gcc
      make install-gcc
      make all-target-libgcc
      make install-target-libgcc
      cd ..
    fi

    cd ..
  fi
fi

if [ "$SKIP_CLEANUP" -eq 0 ]; then
  find "$PREFIX/tmp" -mindepth 1 -maxdepth 1 -exec rm -rf {} \;
fi

echo "Toolchain installed in $PREFIX"
