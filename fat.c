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

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20

#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

struct FatDirent {
    byte name[8];
    byte ext[3];
    byte attr;
    byte res0;
    byte ctimetenth;
    byte ctime[2];
    byte cdate[2];
    byte adate[2];
    byte clusterhi[2];
    byte mtime[2];
    byte mdate[2];
    byte clusterlo[2];
    byte size[4];
};
typedef struct FatDirent FatDirent;

struct Superblock {
    int dev;
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

static u64
min(u64 x, u64 y)
{
    return x < y ? x : y;
}

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
    return sb->volsize - (sb->nresv + (sb->nfats*sb->fatsize) + sb->nrootent*sizeof(FatDirent));
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

    sb->dev = dev;
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
    cprintf("nsect=%d, rootstart=%d\n", nsect(sb), sb->rootstart);

    brelse(b);
}

void
fsinit(int dev)
{
    readsb(dev, &sb);
}

// Inodes in FAT are in-memory FAT direntries. This is because FAT doesn't have
// an on-disk inode and instead stores file metadata on the direntry.
//
// Inum is the byte offset of the direntry on disk (this doesn't hold for the
// root directory, see below).
//
// In FAT32 cluster numbers 0 and 1 are reserved in the FAT, which means cluster
// numbering starts at 2. We use cluster 0 to mean the root directory. Because
// the root directory doesn't have a direntry for itself, we use a fake
// 0x200000 offset to represent the root direntry.

#define ROOTCLUSTER 0
#define ROOTOFFSET  0x200000 // maximum valid directory size

struct Inode {
    uint dev;
    u64 inum;
    int ref;
    SleepLock lock;
    int valid;

    u32 size;
    u32 type;
    u32 firstcluster;

    u32 dircluster;
    u32 diroffset;
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

static void
fat_copy_shortname(Dir *d, FatDirent *fdir)
{
    int i;
    char *s = d->name;

    for (i = 0; i < 8; i++) {
        if (i == 0 && fdir->name[i] == 0x05) {
            *s = 0xE5;
        } else if (fdir->name[i] == 0x20) {
            break;
        } else {
            *s = fdir->name[i];
        }

        s++;
    }

    if (fdir->ext[0] != 0x20) {
        *s = '.';
        s++;
    }


    for (i = 0; i < 3; i++) {
        if (fdir->ext[i] == 0x20) {
            break;
        }

        *s = fdir->ext[i];
        s++;
    }

    *s = '\0';
}

// Cluster is the cluster that contains the dirent.
// Offset is the offset of the dirent in that cluster.
static u64
fat_geninum(u32 cluster, u32 offset)
{
    return (cluster * sb.clustsize * sb.sectsize) + offset;
}

Inode *
iget(uint dev, u32 cluster, u32 offset)
{
    Inode *ip, *empty;
    u64 inum;

    if (cluster == ROOTCLUSTER && sb.isfat32) {
        cluster = sb.rootstart;
    }

    inum = fat_geninum(cluster, offset);

    lock(&icache.lock);

    empty = nil;
    for (ip = &icache.inodes[0]; ip < &icache.inodes[NINODE]; ip++) {
        if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
            ip->ref++;
            unlock(&icache.lock);

            if (ip->dircluster != cluster || ip->diroffset != offset) {
                panic("iget - bad cluster or offset");
            }

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
    ip->dircluster = cluster;
    ip->diroffset = offset;
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

static int
isrootdirent(Inode *ip)
{
    return ((sb.isfat32 && ip->dircluster == sb.rootstart) || ip->dircluster == ROOTCLUSTER) && ip->diroffset == ROOTOFFSET;
}

static u64
cluster2block(u32 cluster, u32 offset)
{
    if (cluster == ROOTCLUSTER) {
        panic("cluster2block - fat16 is not supported");
    }

    if (offset >= sb.clustsize*sb.sectsize) {
        panic("cluster2block - offset to big");
    }

    u64 sector = sb.nresv + (sb.nfats*sb.fatsize) + sb.nrootent*sizeof(FatDirent) + (cluster-2)*sb.clustsize + offset/sb.sectsize;

    return OFFSET + (sector*sb.sectsize)/BSIZE;
}

static void
readcluster(u32 cluster, u32 offset, void *dst, usize n)
{
    usize tot, m;
    byte *dstb = (byte *)dst;
    u64 block;
    Buf *b;

    if (offset + n >= sb.clustsize*sb.sectsize) {
        panic("readcluster - out of range");
    }

    for (tot = 0; tot < n; tot += m, offset += m, dstb += m) {
        block = cluster2block(cluster, offset);
        b = bread(sb.dev, block);
        m = min(n - tot, BSIZE - offset%BSIZE);
        memmove(dstb, b->data+(offset%BSIZE), m);
        brelse(b);
    }
}

void
ilock(Inode *ip)
{
    FatDirent dirent;

    locksleep(&ip->lock);

    if (ip->valid) {
        return;
    }

    if (isrootdirent(ip)) {
        ip->size = 0;
        ip->firstcluster = ip->dircluster;
        ip->type = T_DIR;
    } else {
        readcluster(ip->dircluster, ip->diroffset, &dirent, sizeof(dirent));
        ip->size = read32(dirent.size);
        ip->firstcluster = (read16(dirent.clusterhi) << 16) | read16(dirent.clusterlo);
        ip->type = (dirent.attr & ATTR_DIRECTORY) ? T_DIR : T_FILE;
    }

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

static int
readifile(Inode *ip, void *dst, usize off, usize n)
{
    if (off > ip->size || off + n < off) {
        return -1;
    }
    if (off + n > ip->size) {
        n = ip->size - off;
    }

    return n;
}

static u32
nextcluster(u32 cluster)
{
    u64 offset, blockno;
    u32 next;
    Buf *b;

    if (!sb.isfat32) {
        panic("nextcluster doesn't support fat16 yet");
    }

    // First two cluster numbers are reserved.
    if (cluster == 0 || cluster == 1) {
        return 0;
    }

    cluster -= 2; // clusters start at index 2

    offset = cluster * 4;
    blockno = (sb.nresv*sb.sectsize + offset)/BSIZE;

    b = bread(sb.dev, blockno);
    next = read32(b->data + (offset%BSIZE));
    brelse(b);

    return next;
}

// Skips N dirents that represent files or directories. Will additionally skip
// any empty dirents that aren't at the end of the directory, volume ID dirents,
// and long file name dirents.
//
// Returns the number of bytes skipped on success. If the directory ends before
// we skip N dirents, returns -1.
static int
fat_skipdir(Inode *ip, usize nskip)
{
    if (ip->type != T_DIR) {
        panic("fat_skipdir - must be called on a directory");
    }

    FatDirent fdir;

    u32 cluster = ip->firstcluster;
    usize bpc = sb.clustsize*sb.sectsize; // bytes per cluster
    usize n = nskip * sizeof(FatDirent);
    usize tot;

    for (tot = 0; tot < n; tot += sizeof(FatDirent)) {
        if (cluster == 0) {
            return -1;
        }

        readcluster(cluster, tot % bpc, &fdir, sizeof(FatDirent));

        if (fdir.name[0] == 0) {
            // finished the directory without skipping enough entries
            return -1;
        } else if (fdir.name[0] == 0xE5) {
            // an empty slot, skip it;
            n += sizeof(FatDirent);
        } else if ((fdir.attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) {
            n += sizeof(FatDirent);
        } else if ((fdir.attr & ATTR_VOLUME_ID) == ATTR_VOLUME_ID) {
            n += sizeof(FatDirent);
        }

        if (tot % bpc > (tot+sizeof(FatDirent)) % bpc) {
            cluster = nextcluster(cluster);
        }
    }

    return tot;
}

static int
fat_readdir(Inode *ip, Dir *d, usize offset)
{
    usize bpc = sb.clustsize*sb.sectsize;
    u32 skip = offset/bpc;
    u32 cluster = ip->firstcluster;
    FatDirent fdir;
    usize tot = 0;

    while (skip) {
        cluster = nextcluster(cluster);
        if (cluster == 0) {
            return -1;
        }
        skip--;
    }

    for (;;) {
        if (cluster == 0) {
            return -1;
        }

        readcluster(cluster, offset % bpc, &fdir, sizeof(FatDirent));

        tot += sizeof(FatDirent);
        offset += sizeof(FatDirent);

        if (fdir.name[0] == 0) {
            return -1;
        }

        if (fdir.name[0] != 0xE5 && (fdir.attr & ATTR_LONG_NAME) != ATTR_LONG_NAME && (fdir.attr & ATTR_VOLUME_ID) != ATTR_VOLUME_ID) {
            break;
        }
    }

    d->inum = fat_geninum(cluster, offset);
    fat_copy_shortname(d, &fdir);

    return tot;
}

static int
readdir(Inode *ip, void *dst, usize offset, usize n)
{
    usize ndir, skip, m, tot;
    byte *dstb = (byte *)dst;

    if (offset % sizeof(Dir) || n % sizeof(Dir)) {
        return -1;
    }

    skip = offset/sizeof(Dir);
    ndir = n/sizeof(Dir);

    m = fat_skipdir(ip, skip);

    if (m == -1) {
        return -1;
    }

    for (tot = 0; tot < ndir; tot++, dstb += sizeof(Dir)) {
        m = fat_readdir(ip, (Dir *)dstb, m);

        if (m == -1) {
            break;
        }
    }

    return tot * sizeof(Dir);
}

/*
static int
readidir(Inode *ip, void *dst, usize offset, usize n)
{
    usize bpc = sb.clustsize*sb.sectsize; // bytes per cluster
    usize tot, m;
    u32 skip = offset/bpc;
    u32 cluster = ip->firstcluster;
    byte *dstb = (byte *)dst;

    while (skip) {
        cluster = nextcluster(cluster);
        if (cluster == 0) {
            return -1;
        }
        skip--;
    }

    for (tot = 0; tot < n; tot += m, offset += m, dstb += m) {
        if (cluster == 0) {
            return -1;
        }

        m = min(n - tot, bpc - offset%bpc);
        readcluster(cluster, offset%bpc, dstb, m);
        cluster = nextcluster(cluster);
    }

    return n;
}
*/

int
readi(Inode *ip, void *dst, usize off, usize n)
{
    if (ip->type == T_FILE) {
        return readifile(ip, dst, off, n);
    } else if (ip->type == T_DIR){
        return readdir(ip, dst, off, n);
    } else {
        panic("readi - bad type");
    }
}

Inode *
namei(char *path)
{
    Inode *ip;

    if (*path == '/') {
        ip = iget(ROOTDEV, ROOTCLUSTER, ROOTOFFSET);
    } else {
        panic("namei - not implemented");
    }

    return ip;
}

static void
printfile(char *path)
{
    Inode *ip = namei(path);

    ilock(ip);

    Dir dirent;
    usize offset = 0;
    usize n = 0;

    while ((n = readi(ip, &dirent, offset, sizeof(dirent)))) {
        if (n == -1) {
            break;
        }

        cprintf("%s\n", dirent.name);

        offset += n;
    }

    iunlockput(ip);
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
