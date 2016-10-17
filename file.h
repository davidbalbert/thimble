typedef struct File File;
struct File {
    long (*read)(File *f, char *buf, usize nbytes);
    long (*write)(File *f, char *buf, usize nbytes);

    uint ref;    // reference count
    char *data;
    usize sz;
    usize pos;
    int omode;
};
