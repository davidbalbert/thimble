struct DmaControlBlock {
    u32 ti; // transfer information
    u32 src_addr;
    u32 dest_addr;
    u32 len;
    u32 stride;
    u32 next;
    u32 res[2]; // reserved - set to zero
};
typedef struct DmaControlBlock DmaControlBlock;
