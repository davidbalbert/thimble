#include <stdarg.h>

#define nil ((void *)0)

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef unsigned long  ulong;

typedef unsigned long  uintptr;
typedef unsigned long  usize;

#define nelem(x) (sizeof(x)/sizeof((x)[0]))
