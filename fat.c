#include "u.h"

#include "lock.h"
#include "sleeplock.h"
#include "bio.h"
#include "file.h"
#include "defs.h"
#include "mem.h"

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
#define ATTR_LONG_NAME_MASK (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID | ATTR_DIRECTORY | ATTR_ARCHIVE)

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

struct LongDirent {
    byte seq;
    byte name0[10];
    byte attr;
    byte type;
    byte checksum;
    byte name1[12];
    byte clusterlo[2]; // always 0x0000
    byte name2[4];
};
typedef struct LongDirent LongDirent;

#define LAST_LONGENT 0x40
#define LONGENT_CHARS 13

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
    u32 clustbytes;
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

#define BOFFSET 63

static u32
nsect(Superblock *sb)
{
    return sb->volsize - (sb->nresv + (sb->nfats*sb->fatsize) + sb->nrootent*sizeof(FatDirent));
}

static void
readsb(int dev, Superblock *sb)
{
    Buf *b = bread(dev, BOFFSET);
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

    sb->clustbytes = sb->sectsize * sb->clustsize;

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

    u32 dircluster; // cluster of the directory containing the dirent for this inode
    u32 diroffset;  // offset *within* dircluster
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

// Cluster is the cluster that contains the dirent.
// Offset is the offset of the dirent in that cluster.
static u64
fat_geninum(u32 cluster, u32 offset)
{
    if (offset >= sb.clustbytes) {
        panic("fat_geninum - offset is too big");
    }

    return (cluster * sb.clustbytes) + offset;
}

// Returns an inode from the inode cache respresenting the direntry at
// cluster+offset with ref incremented. Cluster is a cluster within a directory. Offset is the offset
// within cluster.
Inode *
iget(uint dev, u32 cluster, u32 offset)
{
    Inode *ip, *empty;
    u64 inum;

    if (cluster == ROOTCLUSTER && sb.isfat32) {
        cluster = sb.rootstart;
    }

    inum = fat_geninum(cluster, offset%sb.clustbytes);

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

    if (offset >= sb.clustbytes) {
        panic("cluster2block - offset to big");
    }

    u64 sector = sb.nresv + (sb.nfats*sb.fatsize) + sb.nrootent*sizeof(FatDirent)/sb.sectsize + (cluster-2)*sb.clustsize + offset/sb.sectsize;

    return BOFFSET + (sector*sb.sectsize)/BSIZE;
}

static void
readcluster(u32 cluster, u32 offset, void *dst, usize n)
{
    usize tot, m;
    byte *dstb = (byte *)dst;
    u64 block;
    Buf *b;

    if (offset + n >= sb.clustbytes) {
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

// Locks ip, loading inode metadata if necessary.
void
ilock(Inode *ip)
{
    FatDirent fd;

    locksleep(&ip->lock);

    if (ip->valid) {
        return;
    }

    if (isrootdirent(ip)) {
        ip->size = 0;
        ip->firstcluster = ip->dircluster;
        ip->type = T_DIR;
    } else {
        readcluster(ip->dircluster, ip->diroffset, &fd, sizeof(FatDirent));
        ip->size = read32(fd.size);
        ip->firstcluster = (read16(fd.clusterhi) << 16) | read16(fd.clusterlo);
        ip->type = (fd.attr & ATTR_DIRECTORY) ? T_DIR : T_FILE;
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

// reads n bytes into dst starting from cluster, offset by offset.
//
// TODO: deal with n > SSIZE_MAX
static ssize
fatread(Inode *ip, void *dst, usize offset, usize n, u32 *clusterp)
{
    byte *dstb = (byte *)dst;
    usize tot, m;
    u32 cluster = ip->firstcluster;
    usize skip = offset/sb.clustbytes;

    while (skip) {
        cluster = nextcluster(cluster);
        if (cluster == 0) {
            return -1;
        }
        skip--;
    }

    for (tot = 0; tot < n; tot += m, offset += m, dstb += m) {
        // must be at loop start to handle the case where we skip no clusters
        if (cluster == 0) {
            return -1;
        }

        if (clusterp) {
            *clusterp = cluster;
        }

        m = min(n - tot, sb.clustbytes - offset%sb.clustbytes);
        readcluster(cluster, offset % sb.clustbytes, dstb, m);

        cluster = nextcluster(cluster);
    }

    return tot;
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

    return fatread(ip, dst, off, n, nil);
}

static int
islong(FatDirent *fd)
{
    return fd->attr == ATTR_LONG_NAME;
}

// Skips N FatDirents that represent files or directories. Will additionally skip
// any empty dirents that aren't at the end of the directory, volume ID dirents,
// and long file name dirents.
//
// Returns the number of bytes skipped on success. If the directory ends before
// we skip N dirents, returns -1.
static ssize
fat_skipdir(Inode *ip, usize nskip)
{
    if (ip->type != T_DIR) {
        panic("fat_skipdir - must be called on a directory");
    }

    FatDirent fd;
    usize tot, m;
    usize n = nskip * sizeof(FatDirent);

    for (tot = 0; tot < n; tot += m) {
        m = fatread(ip, &fd, tot, sizeof(FatDirent), nil);

        if (m == -1) {
            // m will be -1 if offset or offset+n is past the last cluster in
            // the list.
            return -1;
        } else if (fd.name[0] == 0) {
            // finished the directory without skipping enough entries
            return -1;
        } else if (fd.name[0] == 0xE5) {
            // an empty slot, skip it;
            n += sizeof(FatDirent);
        } else if (islong(&fd)) {
            n += sizeof(FatDirent);
        } else if ((fd.attr & ATTR_VOLUME_ID) == ATTR_VOLUME_ID) {
            n += sizeof(FatDirent);
        }
    }

    return tot;
}

static char
fat_downcase(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    } else {
        return c;
    }
}

static void
copy_shortname(Dirent *d, FatDirent *fd)
{
    int i;
    char *s = d->name;

    for (i = 0; i < 8; i++) {
        if (i == 0 && fd->name[i] == 0x05) {
            *s = 0xE5;
        } else if (fd->name[i] == 0x20) {
            break;
        } else {
            *s = fat_downcase(fd->name[i]);
        }

        s++;
    }

    if (fd->ext[0] != 0x20) {
        *s = '.';
        s++;
    }


    for (i = 0; i < 3; i++) {
        if (fd->ext[i] == 0x20) {
            break;
        }

        *s = fat_downcase(fd->ext[i]);
        s++;
    }

    *s = '\0';
}

// Returns the number of bytes written. Sets nrune to the number of runes read.
static int
copy_codepoints(Dirent *d, int charoff, byte *codepoints, int count, int *nrune)
{
    int i, n, tot = 0;
    Rune r;

    for (i = 0; i < count; i++) {
        r = read16(codepoints + i*2);

        if (r == 0 || charoff + runelen(r) > DIRSIZ) {
            if (charoff < DIRSIZ) {
                d->name[charoff] = '\0';
                tot += 1;
            }

            break;
        }

        n = runetochar(d->name + charoff, &r);

        charoff += n;
        tot += n;
    }

    *nrune = i;
    return tot;
}

// Reads the LFN starting at fd into d->name
//
// - fd is the first LongDirent in the series
// - offset is the offset of the **next** LongDirent
//
// Returns the number of additional bytes read.
//
// After execution:
//
// - fd is the associated FatDirent.
// - cluster is the cluster that contains the final fd.
static ssize
read_longname(Inode *ip, Dirent *d, FatDirent *fd, usize offset, u32 *clusterp)
{
    LongDirent *ld = (LongDirent *)fd;

    if (!(ld->seq & LAST_LONGENT)) {
        panic("read_longname - fd is not the last longent");
    }

    int nlents = ld->seq & ~LAST_LONGENT;
    usize n;

    LongDirent lds[nlents+1]; // the last element of lds is a regular FatDirent
    lds[0] = *ld;

    n = fatread(ip, lds+1, offset, nlents*sizeof(LongDirent), clusterp);

    if (n == -1) {
        return -1;
    }

    int nrune = 0, charoff = 0;
    for (ld = &lds[nlents-1]; ld >= lds; ld--) {
        charoff += copy_codepoints(d, charoff, ld->name0, 5, &nrune);
        if (nrune < 5) { break; }

        charoff += copy_codepoints(d, charoff, ld->name1, 6, &nrune);
        if (nrune < 6) { break; }

        charoff += copy_codepoints(d, charoff, ld->name2, 2, &nrune);
        if (nrune < 2) { break; }
    }

    *fd = *((FatDirent *)&lds[nlents]);

    return n;
}


// Reads a Dirent from a FAT directory, consuming any number of FatDirents
// (volume id, empty entries, and long file name entries). Returns -1 if we
// reach the end of a directory before reading a Dirent. Otherwise, returns the
// number of bytes consumed from the FAT directory.
static ssize
fat_readdir(Inode *ip, Dirent *d, usize offset)
{
    FatDirent fd;
    usize tot, n;
    u32 cluster;

    tot = 0;
    for (;;) {
        n = fatread(ip, &fd, offset, sizeof(FatDirent), &cluster);

        tot += n;
        offset += n;

        if (fd.name[0] == 0) {
            return -1;
        }

        if (fd.name[0] != 0xE5 && (!(fd.attr & ATTR_VOLUME_ID) || islong(&fd))) {
            break;
        }
    }

    if (islong(&fd)) {
        n = read_longname(ip, d, &fd, offset, &cluster);

        if (n == -1) {
            return -1;
        }

        tot += n;
        offset += n;
    } else {
        copy_shortname(d, &fd);
    }

    d->inum = fat_geninum(cluster, offset%sb.clustbytes);
    d->dircluster = cluster;

    // offset was incremented past the FatDirent, so move it backwards
    d->diroffset = (offset-sizeof(FatDirent))%sb.clustbytes;

    return tot;
}

static ssize
readdir(Inode *ip, void *dst, usize offset, usize n)
{
    usize ndir, skip, m, tot;
    byte *dstb = (byte *)dst;

    if (offset % sizeof(Dirent) || n % sizeof(Dirent)) {
        return -1;
    }

    skip = offset/sizeof(Dirent);
    ndir = n/sizeof(Dirent);

    m = fat_skipdir(ip, skip);

    if (m == -1) {
        return -1;
    }

    for (tot = 0; tot < ndir; tot++, dstb += sizeof(Dirent)) {
        m = fat_readdir(ip, (Dirent *)dstb, m);

        if (m == -1) {
            break;
        }
    }

    return tot * sizeof(Dirent);
}

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

//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
//
static char*
skipelem(char *path, char *name)
{
    char *s;
    int len;

    while (*path == '/') {
        path++;
    }
    if (*path == '\0') {
        return nil;
    }

    s = path;
    while (*path != '/' && *path != '\0') {
        path++;
    }
    len = path - s;

    if (len >= DIRSIZ) {
        memmove(name, s, DIRSIZ);
    } else {
        memmove(name, s, len);
        name[len] = '\0';
    }

    while (*path == '/') {
        path++;
    }

    return path;
}

static int
namecmp(char *a, char *b)
{
    return strncmp(a, b, DIRSIZ);
}

Inode *
dirlookup(Inode *dp, char *name)
{
    Dirent d;
    usize off = 0;

    if (dp->type != T_DIR) {
        panic("dirlookup - dp is not a directory");
    }

    for (;;) {
        if (readi(dp, &d, off, sizeof(Dirent)) != sizeof(Dirent)) {
            return nil;
        }

        if (namecmp(name, d.name) == 0) {
            return iget(dp->dev, d.dircluster, d.diroffset);
        }

        off += sizeof(Dirent);
    }

    return nil;
}

// Retruns an unlocked inode representing the file at path, or nil if the file
// can't be found.
Inode *
namei(char *path)
{
    Inode *ip, *next;
    char name[DIRSIZ];

    if (*path == '/') {
        ip = iget(ROOTDEV, ROOTCLUSTER, ROOTOFFSET);
    } else {
        panic("namei - cwd not implemented");
    }

    while ((path = skipelem(path, name)) != nil) {
        ilock(ip);

        if (ip->type != T_DIR) {
            iunlockput(ip);
            return nil;
        }

        if ((next = dirlookup(ip, name)) == nil) {
            iunlockput(ip);
            return nil;
        }

        iunlockput(ip);
        ip = next;
    }

    return ip;
}

static void
printfile(char *path)
{
    Inode *ip = namei(path);
    usize n;

    if (ip == nil) {
        panic("printfile - no file named %s", path);
    }

    ilock(ip);

    if (ip->size > PGSIZE) {
        panic("printfile - ip->size too big");
    }

    byte *data = kalloc();
    if (data == nil) {
        panic("printfile - can't allocate page");
    }

    if ((n = readi(ip, data, 0, ip->size)) != ip->size) {
        panic("coudln't read data");
    }

    cprintf("%s", data);

    kfree(data);

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
