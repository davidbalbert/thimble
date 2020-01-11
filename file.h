#define ROOTDEV 1

#define T_DIR  1
#define T_FILE 2

typedef struct File File;
struct File {
    long (*read)(File *f, char *buf, usize nbytes);
    long (*write)(File *f, char *buf, usize nbytes);

    u32 ref;    // reference count
    char *data;
    usize sz;
    usize pos;
    int omode;
};

// Filenames are a maximum of DIRSIZ long. If the length == DIRSIZ, name is not
// null terminated. If length < DIRSIZ, name is null terminated.
#define DIRSIZ 256

struct Dirent {
    u64 inum;
    char name[DIRSIZ]; // a UTF-8 string
};
typedef struct Dirent Dirent;
