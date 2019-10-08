#echo + set arch i386:x86-64\n
#set arch i386:x86-64

echo + file kernel\n
file kernel

echo + symbol-file kernel\n
symbol-file kernel

echo + target remote localhost:1234\n
target remote localhost:1234
