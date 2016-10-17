#include "u.h"

#include "defs.h"
#include "file.h"
#include "lib.h"
#include "proc.h"

#define NFILE 2048
static File files[NFILE];

static char *hello = "Hello Thimble from a file!";

static long
readfile(File *f, char *buf, usize nbytes)
{
    if (f->sz < nbytes)
        nbytes = f->sz;

    memmove(buf, f->data, nbytes);
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

    for (f = &files[0]; f < &files[nelem(files)]; f++) {
        if (f->ref == 0) {
            f->ref++;
            return f;
        }
    }

    return nil;
}

static void
initfile(File *f, char *data, usize size)
{
    f->read = readfile;
    f->write = writefile;
    f->data = data;
    f->sz = size;
}

static long
readcons(File *f, char *buf, usize nbytes)
{
    return -1;
}

static long
writecons(File *f, char *buf, usize nbytes)
{
    char s[nbytes+1];

    memmove(s, buf, nbytes);
    s[nbytes] = '\0';

    cprintf("%s", s);

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
freefile(File *f)
{
    f->read = nil;
    f->write = nil;

    f->ref = 0;
    f->data = nil;
    f->sz = 0;
    f->pos = 0;
}

static void
releasefile(File *f)
{
    if (--f->ref == 0)
        freefile(f);
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
        freefile(f);
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
sys_open(SyscallFrame *f)
{
    char *fname;
    long l;
    int omode, fd;
    File *file;

    if (argstr(f, 0, &fname) < 0)
        return -1;

    if (arglong(f, 1, &l) < 0)
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
        freefile(file);
        return -1;
    }

    return fd;
}

long
sys_close(SyscallFrame *f)
{
    int fd;
    File *file;

    if (argfd(f, 0, &fd)) {
        // todo errstr
        return -1;
    }

    file = proc->files[fd];

    releasefile(file);
    proc->files[fd] = nil;

    return 0;
}

long
sys_read(SyscallFrame *f)
{
    int fd;
    long l;
    uintptr buf;
    usize nbytes;
    File *file;

    // fd
    if (argfd(f, 0, &fd) < 0)
        return -1;

    // nbytes (needed for argptr)
    if (arglong(f, 2, &l) < 0) {
        // todo errstr
        return -1;
    }
    nbytes = (usize)l;

    // buf
    if (argptr(f, 1, &buf, nbytes) < 0) {
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
sys_write(SyscallFrame *f)
{
    int fd;
    long l;
    uintptr buf;
    usize nbytes;
    File *file;

    // fd
    if (argfd(f, 0, &fd) < 0)
        return -1;

    // nbytes (needed for argptr)
    if (arglong(f, 2, &l) < 0) {
        // todo errstr
        return -1;
    }
    nbytes = (usize)l;

    // buf
    if (argptr(f, 1, &buf, nbytes) < 0) {
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
