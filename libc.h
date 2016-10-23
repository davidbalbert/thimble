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

#define RFPROC   (1<<0)  // new proc
#define RFNOWAIT (1<<1)  // dissasociate child from parent (no Waitmsg for parent)
#define RFNAMEG  (1<<2)  // copy rather than share namespace
#define RFCNAMEG (1<<3)  // create clean namespace
#define RFNOMNT  (1<<4)	 // no new mounts and no paths beginning with #
#define RFENVG   (1<<5)  // copy rather than share environmental variables
#define RFCENVG  (1<<6)  // create empty environment
#define RFNOTEG  (1<<7)  // create new note group
#define RFFDG    (1<<8)  // copy rather than share file descriptor table
#define RFCFDG   (1<<9)  // create empty file descriptor table
#define RFREND   (1<<10) // create new rendevouz group
#define RFMEM    (1<<11) // share data and bss segments

int fork(void);
int rfork(int flags);

void *memmove(void *dst, void *src, usize n);
void *memset(void *p, int c, usize n);
void *memzero(void *p, usize n);
int isdigit(int c);
int strcmp(char *s1, char *s2);
usize strlen(char *s);
long strtol(char *s, char **endptr, int base);
