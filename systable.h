long sys_open(void);
long sys_close(void);
long sys_read(void);
long sys_write(void);
long sys_rfork(void);

static long (*syscalls[])(void) = {
    [SYS_OPEN] sys_open,
    [SYS_CLOSE] sys_close,
    [SYS_READ] sys_read,
    [SYS_WRITE] sys_write,
    [SYS_RFORK] sys_rfork,
};
