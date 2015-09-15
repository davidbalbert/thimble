// base - 32 bits
// limit - 20 bits
// access - 8 bits
// flags - 4 bits
#define GDT_ENTRY(base, limit, access, flags)               \
    .word   limit & 0xFFFF;                                 \
    .word   base & 0xFFFF;                                  \
    .byte   base >> 16 & 0xFF;                              \
    .byte   access;                                         \
    .byte   (flags << 4 & 0xF0) | (limit >> 16 & 0x0F);     \
    .byte   base >> 24 & 0xFF;
