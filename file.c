#include "u.h"

#include "archdefs.h"
#include "defs.h"
#include "file.h"
#include "lib.h"
#include "lock.h"
#include "proc.h"

#define NFILE 100

static struct {
    SpinLock lock;
    File files[NFILE];
} ftable;

static char *hello = "Hello Thimble from a file!";

static long
readfile(File *f, char *buf, usize nbytes)
{
    if (f->sz < nbytes)
        nbytes = f->sz;

    memmove(buf, f->data, nbytes);
    f->pos += nbytes;

    return nbytes;
}

static long
writefile(File *f, char *buf, usize nbytes)
{
    // TODO: set errstr
    return -1;
}

static File *
allocfile(void)
{
    File *f;

    lock(&ftable.lock);
    for (f = ftable.files; f < &ftable.files[nelem(ftable.files)]; f++) {
        if (f->ref == 0) {
            f->ref++;
            unlock(&ftable.lock);
            return f;
        }
    }
    unlock(&ftable.lock);

    return nil;
}

static void
initfile(File *f, char *data, usize size)
{
    f->read = readfile;
    f->write = writefile;
    f->data = data;
    f->sz = size;
    f->pos = 0;
}

static long
readcons(File *f, char *buf, usize nbytes)
{
    return -1;
}

static long
writecons(File *f, char *buf, usize nbytes)
{
    cwrite(buf, nbytes);
    return nbytes;
}

static void
consfile(File *f)
{
    f->read = readcons;
    f->write = writecons;
}

static int
allocfd(Proc *p, File *f)
{
    int fd;

    if (p->nextfd >= NFD)
        return -1;

    fd = p->nextfd++;

    p->files[fd] = f;

    return fd;
}

static void
releasefile(File *f)
{
    lock(&ftable.lock);

    f->ref--;
    if (f->ref > 0) {
        unlock(&ftable.lock);
        return;
    }

    f->read = nil;
    f->write = nil;

    f->ref = 0;
    f->data = nil;
    f->sz = 0;
    f->pos = 0;

    unlock(&ftable.lock);
}

static File *
retainfile(File *f)
{
    lock(&ftable.lock);
    f->ref++;
    unlock(&ftable.lock);

    return f;
}

static File *
getfile(char *fname)
{
    File *f = allocfile();
    if (f == nil)
        return nil;

    if (strcmp(fname, "/hello.txt") == 0) {
        initfile(f, hello, strlen(hello));
    } else if (strcmp(fname, "/dev/cons") == 0) {
        consfile(f);
    } else {
        releasefile(f);
        f = nil;
    }

    return f;
}

static int
canwrite(char *fname)
{
    return strcmp(fname, "/dev/cons") == 0;
}

int
sys_open(void)
{
    char *fname;
    long l;
    int omode, fd;
    File *file;

    if (argstr(0, &fname) < 0)
        return -1;

    if (arglong(1, &l) < 0)
        return -1;

    omode = (long)l;

    if (omode & (OEXEC | OTRUNC | OCEXEC | ORCLOSE)) {
        // todo errstr
        return -1;
    }

    if (omode & (OWRITE | ORDWR) && !canwrite(fname)) {
        // todo errstr
        return -1;
    }

    file = getfile(fname);
    if (file == nil)
        return -1;

    file->omode = omode;

    fd = allocfd(proc, file);

    if (fd == -1) {
        // todo errstr
        releasefile(file);
        return -1;
    }

    return fd;
}

long
sys_close(void)
{
    int fd;
    File *file;

    if (argfd(0, &fd)) {
        // todo errstr
        return -1;
    }

    file = proc->files[fd];

    releasefile(file);
    proc->files[fd] = nil;

    return 0;
}

long
sys_read(void)
{
    int fd;
    long l;
    uintptr buf;
    usize nbytes;
    File *file;

    // fd
    if (argfd(0, &fd) < 0)
        return -1;

    // nbytes (needed for argptr)
    if (arglong(2, &l) < 0) {
        // todo errstr
        return -1;
    }
    nbytes = (usize)l;

    // buf
    if (argptr(1, &buf, nbytes) < 0) {
        // todo errstr
        return -1;
    }

    file = proc->files[fd];

    if (!(file->omode & (OREAD | ORDWR))) {
        // todo errstr
        return -1;
    }

    return file->read(file, (char *)buf, nbytes);
}

long
sys_write(void)
{
    int fd;
    long l;
    uintptr buf;
    usize nbytes;
    File *file;

    // fd
    if (argfd(0, &fd) < 0)
        return -1;

    // nbytes (needed for argptr)
    if (arglong(2, &l) < 0) {
        // todo errstr
        return -1;
    }
    nbytes = (usize)l;

    // buf
    if (argptr(1, &buf, nbytes) < 0) {
        // todo errstr
        return -1;
    }

    file = proc->files[fd];

    if (!(file->omode &(OWRITE | ORDWR))) {
        // todo errstr
        return -1;
    }

    return file->write(file, (char *)buf, nbytes);
}

void
copyfds(Proc *oldp, Proc *newp)
{
    int i;
    File *f;

    for (i = 0; i < NFD; i++) {
        f = oldp->files[i];

        if (f == nil)
            continue;

        newp->files[i] = retainfile(f);
    }

    newp->nextfd = oldp->nextfd;
}

void
fileinit(void)
{
    initlock(&ftable.lock);
}
