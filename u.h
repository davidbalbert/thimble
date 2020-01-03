#include <stdarg.h>

#define nil ((void *)0)

typedef unsigned char  byte;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  u64;

typedef unsigned long uintptr;
typedef unsigned long usize;

typedef long ssize;

typedef unsigned int uint;

typedef unsigned long Pte;

#define nelem(x) (sizeof(x)/sizeof((x)[0]))
