// Definitions from libc used in the kernel

#define OREAD   (1<<0)
#define OWRITE  (1<<1)
#define ORDWR   (1<<2)
#define OEXEC   (1<<3)

#define OTRUNC  (1<<4) // truncate before opening
#define OCEXEC  (1<<5) // close on exec
#define ORCLOSE (1<<6) // remove on close
