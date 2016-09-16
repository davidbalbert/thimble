#!/usr/bin/env bash

if [ "$0" != "$BASH_SOURCE" ]; then
  echo "setup.bash is not for sourcing. You probably meant to source env.bash." >&2
  return
fi

set -e

GMP=https://ftp.gnu.org/gnu/gmp/gmp-6.1.1.tar.bz2
MPFR=https://ftp.gnu.org/gnu/mpfr/mpfr-3.1.4.tar.bz2
MPC=https://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz
BINUTILS=https://ftp.gnu.org/gnu/binutils/binutils-2.27.tar.bz2
GCC=https://ftp.gnu.org/gnu/gcc/gcc-6.2.0/gcc-6.2.0.tar.bz2

export MAKEFLAGS=-j4

function envfile() {
  pushd $(dirname $0) >/dev/null
  echo $(pwd)/env.bash
  popd >/dev/null
}

. $(envfile)

function get() {
  local name

  name=$(fname $1)

  echo Downloading ${name}...
  curl -# -o $name $1

  echo Extracting...
  tar xf $name
  rm $name
}

function fname() {
  echo $1 | sed 's/^.*\///g'
}

function dir() {
  echo $(fname $1) | sed 's/\.tar\.bz2$//' | sed 's/\.tar\.gz$//'
}

mkdir -p $PREFIX

if [ -z "$NODOWNLOAD" ]; then
  get $GMP
  get $MPFR
  get $MPC
  get $BINUTILS
  get $GCC
  git clone https://github.com/davidbalbert/qemu.git -b 6.828-2.7.0
fi

cd $(dir $GMP)
./configure --prefix=$PREFIX
make
make install
cd ..

cd $(dir $MPFR)
./configure --prefix=$PREFIX --with-gmp=$PREFIX
make
make install
cd ..

cd $(dir $MPC)
./configure --prefix=$PREFIX --with-gmp=$PREFIX
make
make install
cd ..

cd $(dir $BINUTILS)
./configure --prefix=$PREFIX --target=x86_64-elf --disable-werror
make
make install
cd ..

cd $(dir $GCC)
mkdir build
cd build
../configure --prefix=$PREFIX --target=x86_64-elf --disable-werror \
   --disable-libssp --disable-libmudflap --with-newlib \
   --without-headers --enable-languages=c --with-gmp=$PREFIX
make all-gcc
make install-gcc
make all-target-libgcc
make install-target-libgcc
cd ../../

if [ "$(uname)" == "Darwin" ]; then
  QEMU_CONFIG_OPTS="--disable-gtk"
else
  QEMU_CONFIG_OPTS=""
fi

cd qemu
./configure --disable-kvm --prefix=$PREFIX --target-list=x86_64-softmmu $QEMU_CONFIG_OPTS
make
make install
cd ..

if [ -z "$NORM" ]; then
  rm -rf $(dir $GMP)
  rm -rf $(dir $MPFR)
  rm -rf $(dir $MPC)
  rm -rf $(dir $BINUTILS)
  rm -rf $(dir $GCC)
  rm -rf qemu
fi
