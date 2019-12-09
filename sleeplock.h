struct SleepLock {
    uint locked;
    SpinLock lock;

    char *name;
    int pid;
};
typedef struct SleepLock SleepLock;
