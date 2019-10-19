struct Cpu {
    int id;                         // cpu id number
    int ncli;                       // number of times pushcli has been called.
    int intena;                     // whether interrupts were enabled before pushcli
};
typedef struct Cpu Cpu;

extern Cpu  *cpu;   // current cpu
