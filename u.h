#include <stdarg.h>

#define nil ((void *)0)

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef unsigned long  ulong;

typedef uchar  u8;
typedef ushort u16;
typedef uint   u32;
typedef ulong  u64;

typedef unsigned long  uintptr;
typedef unsigned long  usize;

#define nelem(x) (sizeof(x)/sizeof((x)[0]))
