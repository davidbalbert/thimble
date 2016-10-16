#include "u.h"

#include "defs.h"
#include "lib.h"
#include "proc.h"

typedef struct File File;
struct File {
    long (*read)(File *f, void *buf, usize nbytes);
    long (*write)(File *f, void *buf, usize nbytes);

    int ref;    // reference count
    char *data;
    usize sz;
    usize pos;
    int omode;
};

#define NFILE 2048

static File files[NFILE];

static char *hello = "Hello Thimble from a file!";

static long
readfile(File *f, void *buf, usize nbytes)
{
    return -1;
}

static long
writefile(File *f, void *buf, usize nbytes)
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
            f->read = readfile;
            f->write = writefile;
        }
    }

    return nil;
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

static File *
getfile(char *fname)
{
    File *f = allocfile();
    if (f == nil)
        return nil;

    if (strcmp(fname, "/hello.txt") == 0) {
        f->data = hello;
        f->sz = strlen(hello);
        return f;
    } else {
        freefile(f);
        return nil;
    }
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
    if (f == nil)
        return -1;

    fd = allocfd(proc, file);

    if (fd == -1) {
        // todo errstr
        freefile(file);
        return -1;
    }

    return fd;
}

