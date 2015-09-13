#!/bin/bash

GMP=https://ftp.gnu.org/gnu/gmp/gmp-6.0.0a.tar.bz2
MPFR=https://ftp.gnu.org/gnu/mpfr/mpfr-3.1.3.tar.bz2
MPC=https://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz
BINUTILS=https://ftp.gnu.org/gnu/binutils/binutils-2.25.1.tar.bz2
GCC=https://ftp.gnu.org/gnu/gcc/gcc-5.2.0/gcc-5.2.0.tar.bz2

set -e

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
  curl -o $name $1

  echo Extracting ${name}...
  tar xf $name
  rm $name

  echo
  echo
}

function fname() {
  echo $1 | sed 's/^.*\///g'
}

function dir() {
  # last sed is a hack for gmp-6.0.0a, whose directory is gmp-6.0.0
  echo $(fname $1) | sed 's/\.tar\.bz2$//' | sed 's/\.tar\.gz$//' | sed 's/a$//'
}

mkdir -p $PREFIX

get $GMP
get $MPFR
get $MPC
get $BINUTILS
get $GCC
git clone https://github.com/geofft/qemu.git -b 6.828-2.3.0

cd $(dir $GMP)
./configure --prefix=$PREFIX
make
make install
cd ..

cd $(dir $MPFR)
./configure --prefix=$PREFIX
make
make install
cd ..

cd $(dir $MPC)
./configure --prefix=$PREFIX
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
   --without-headers --enable-languages=c
make all-gcc
make install-gcc
make all-target-libgcc
make install-target-libgcc
cd ../../

cd qemu
./configure --disable-kvm --prefix=$PREFIX --target-list=x86_64-softmmu
make
make install
cd ..

rm -rf $(dir $GMP)
rm -rf $(dir $MPFR)
rm -rf $(dir $MPC)
rm -rf $(dir $BINUTILS)
rm -rf $(dir $GCC)
rm -rf qemu
