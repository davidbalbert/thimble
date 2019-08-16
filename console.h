struct Console {
    void (*clear)(void);
    void (*puts)(char *s);
    void (*putc)(uchar c);
};

typedef struct Console Console;
