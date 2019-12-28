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
