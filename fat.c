#include "u.h"

#include "lock.h"
#include "sleeplock.h"
#include "bio.h"
#include "file.h"
#include "defs.h"

struct Bpb {
    byte jmpboot[3];
    byte name[8];
    byte sectsize[2];  // bytes
    byte clustsize;    // sectors
    byte nresv[2];     // sectors
    byte nfats;
    byte nrootent[2];  // root directroy entries (0 on FAT32)
    byte volsize[2];   // sectors
    byte mediatype;
    byte fatsize[2];   // sectors
    byte trksize[2];
    byte nheads[2];
    byte nhidden[4];
    byte volsize32[4]; // sectors, shared with Bpb32 up to here
    byte driveno;
    byte res0;
    byte bootsig;
    byte volid[4];
    byte label[11];
    byte fstype[8];
};
typedef struct Bpb Bpb;

struct Bpb32 {
    byte jmpboot[3];
    byte name[8];
    byte sectsize[2];   // bytes
    byte clustsize;     // sectors
    byte nresv[2];      // sectors
    byte nfats;
    byte nrootent[2];   // root directroy entries (0 on FAT32)
    byte volsize[2];    // sectors
    byte mediatype;
    byte fatsize[2];    // sectors
    byte trksize[2];
    byte nheads[2];
    byte nhidden[4];
    byte volsize32[4];  // last attribute shared with Bpb
    byte fatsize32[4];  // sectors
    byte extflags[2];
    byte version[2];
    byte rootstart[4];  // first cluster of root directory
    byte infosect[2];
    byte backupboot[2]; // sector number of backup boot sector (0 or 6)
    byte res0[12];
    byte driveno;
    byte res1;
    byte bootsig;
    byte volid[4];
    byte label[11];
    byte fstype[8];
};
typedef struct Bpb32 Bpb32;

struct Superblock {
    int isfat32;
    u16 sectsize;
    byte clustsize;
    u16 nresv;
    byte nfats;
    u16 nrootent;
    u32 volsize;
    u32 fatsize;
    u32 rootstart;
    u16 infosect;
};
typedef struct Superblock Superblock;
Superblock sb;

static u16
read16(byte data[2])
{
    return (data[0]<<0) | (data[1]<<8);
}

static u32
read32(byte data[4])
{
    return (data[0]<<0) | (data[1]<<8) | (data[2] << 16) | (data[3] << 24);
}

#define OFFSET 63

static u32
nsect(Superblock *sb)
{
    return sb->volsize - (sb->nresv + (sb->nfats*sb->fatsize) + sb->nrootent);
}

static u32
ncluster(Superblock *sb)
{
    return nsect(sb) / sb->clustsize;
}

static void
readsb(int dev, Superblock *sb)
{
    Buf *b = bread(dev, OFFSET);
    Bpb *bpb = (Bpb *)b->data;

    u16 fatsize = read16(bpb->fatsize);

    if (fatsize == 0) {
        sb->isfat32 = 1;
    } else {
        panic("readsb - fat16 is not supported");
    }

    Bpb32 *bpb32 = (Bpb32 *)bpb;

    sb->sectsize = read16(bpb32->sectsize);
    sb->clustsize = bpb32->clustsize;
    sb->nresv = read16(bpb32->nresv);
    sb->nfats = bpb32->nfats;
    sb->nrootent = read16(bpb32->nrootent);
    sb->fatsize = read32(bpb32->fatsize32);

    if ((sb->volsize = read16(bpb32->volsize)) == 0) {
        sb->volsize = read32(bpb32->volsize32);
    }

    sb->rootstart = read32(bpb32->rootstart);
    sb->infosect = read16(bpb32->infosect);

    cprintf("fatsize=%d, isfat32=%d, sectsize=%d, clustsize=%d\n", fatsize, sb->isfat32, sb->sectsize, sb->clustsize);
    cprintf("nsect=%d, ncluster=%d\n", nsect(sb), ncluster(sb));

    brelse(b);
}

void
fsinit(int dev)
{
    readsb(dev, &sb);
}


struct Inode {
    uint dev;
    u64 inum;
    int ref;
    SleepLock lock;
    int valid;

    u32 size;
};
typedef struct Inode Inode;

#define NINODE 50

struct {
    SpinLock lock;
    Inode inodes[NINODE];
} icache;

void
iinit(void)
{
    int i;

    initlock(&icache.lock, "icache");
    for (i = 0; i < NINODE; i++) {
        initsleeplock(&icache.inodes[i].lock, "inode");
    }
}

Inode *
iget(uint dev, u64 inum)
{
    Inode *ip, *empty;

    lock(&icache.lock);

    empty = nil;
    for (ip = &icache.inodes[0]; ip < &icache.inodes[NINODE]; ip++) {
        if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
            ip->ref++;
            unlock(&icache.lock);
            return ip;
        }

        if (empty == nil && ip->ref == 0) {
            empty = ip;
        }
    }

    if (empty == nil) {
        panic("iget - out of inodes");
    }

    ip = empty;
    ip->dev = dev;
    ip->inum = inum;
    ip->ref = 1;
    ip->valid = 0;
    unlock(&icache.lock);

    return ip;
}

void
iput(Inode *ip)
{
    if (ip->ref == 0) {
        panic("iput");
    }

    lock(&icache.lock);
    ip->ref--;
    unlock(&icache.lock);
}

void
ilock(Inode *ip)
{
    locksleep(&ip->lock);

    ip->size = 512;
    ip->valid = 1;
}

void
iunlock(Inode *ip)
{
    if (!holdingsleep(&ip->lock)) {
        panic("iunlock");
    }

    unlocksleep(&ip->lock);
}

void
iunlockput(Inode *ip)
{
    iunlock(ip);
    iput(ip);
}

int
readi(Inode *ip, void *dst, usize off, usize n)
{
    memzero(dst, n);

    return n;
}

Inode *
namei(char *path)
{
    Inode *ip;

    if (*path == '/') {
        ip = iget(ROOTDEV, ROOTINO);
    }

    return ip;
}

static void
printfile(char *path)
{
    Inode *ip = namei(path);

    ilock(ip);

    byte data[ip->size];
    readi(ip, data, 0, ip->size);

    iunlockput(ip);

    xxd(data, ip->size, 0);
}

long
sys_printfile(void)
{
    char *path;

    if (argstr(0, &path) < 0) {
        return -1;
    }

    printfile(path);

    return 0;
}
