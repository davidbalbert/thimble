#define BSIZE 1024

typedef struct Buf Buf;
struct Buf {
    SpinLock lock;
    int valid;
    int disk;
    uint dev;
    u64 blockno;
    uint refcnt;
    byte data[BSIZE];
};
