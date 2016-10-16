long hello(int a, int b, int c, int d, int e, int f);
long goodbye(int a, int b, int c, int d, int e, int f);
void print(char *s);
void printlong(long l);

#define OREAD   (1<<0)
#define OWRITE  (1<<1)
#define ORDWR   (1<<2)
#define OEXEC   (1<<3)

#define OTRUNC  (1<<4) // truncate before opening
#define OCEXEC  (1<<5) // close on exec
#define ORCLOSE (1<<6) // remove on close

int open(char *file, int omode);
int close(int fd);
long read(int fd, void *buf, usize nbytes);
long write(int fd, void *buf, usize nbytes);


void *memmove(void *dst, void *src, usize n);
void *memset(void *p, int c, usize n);
void *memzero(void *p, usize n);
int isdigit(int c);
int strcmp(char *s1, char *s2);
usize strlen(char *s);
long strtol(char *s, char **endptr, int base);
