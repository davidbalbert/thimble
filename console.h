struct Console {
    void (*clear)(void);
    void (*puts)(char *s);
    void (*putc)(byte c);
};

typedef struct Console Console;
