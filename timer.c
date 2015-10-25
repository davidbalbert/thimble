#include "types.h"

#include "common.h"
#include "irq.h"
#include "x86.h"

#define TIMER_FREQ 1193182

// Calculate the reload value for a given desired frequency x.
// Add x/2 to TIMER_FREQ so that we round correctly.
#define TIMER_RELOAD(x) ((TIMER_FREQ+(x)/2)/(x))

// IO ports
#define TIMER_CHAN0 0x40
#define TIMER_CMD 0x43

#define TIMER_SEL0 (0 << 6)        // Select channel 0
#define TIMER_LOHI (0b11 << 4)     // 16 bit mode
#define TIMER_FREQDIV (2 << 1)     // repeating timer

void
timerinit(void)
{
    outb(TIMER_CMD, TIMER_SEL0 | TIMER_LOHI | TIMER_FREQDIV);
    outb(TIMER_CHAN0, TIMER_RELOAD(100) & 0xFF);
    outb(TIMER_CHAN0, TIMER_RELOAD(100) >> 8);
    picenable(IRQ_TIMER);
}
