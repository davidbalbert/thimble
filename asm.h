// base - 32 bits
// limit - 20 bits
// access - 8 bits
// flags - 4 bits
#define GDT_ENTRY(base, limit, access, flags) \
    .word   limit & 0xFFFF      # First 2 bytes of limit \
    .word   base & 0xFFFF       # First 2 bytes of base \
    .byte   base >> 16 & 0xFF   # Third byte of base \
    .byte   access              # Access byte \
    .byte   (flags << 4 & 0xF0) | (limit >> 16 & 0x0F)    # 4 bits of flags, last 4 bits of limit \
    .byte   base >> 24 & 0xFF   # 4th byte of base
    
