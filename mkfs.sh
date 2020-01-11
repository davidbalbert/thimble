#!/bin/sh

set -e

name=$1

dd bs=512 count=131072 if=/dev/zero of=$name 2> /dev/null
fdisk -iy -a dos $name 2> /dev/null

disk=$(hdiutil attach -nomount sd.img | head -n1 | awk '{ print $1 }')

newfs_msdos -F 32 -v thimble $(printf "%ss1" $disk) > /dev/null 2>&1

hdiutil detach $disk > /dev/null

output=$(hdiutil attach $name)

disk=$(echo $output | awk '{ print $1 }');
volume=$(echo $output | awk '{ print $5 }');

cp kernel8.img $volume/kernel8.img
cp héllo.txt $volume/héllo.txt

hdiutil detach $disk > /dev/null
