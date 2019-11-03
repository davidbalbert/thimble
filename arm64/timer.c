// Broadcom 2837 system timer

#include "u.h"

#include "arm64.h"
#include "defs.h"
#include "irq.h"
#include "mem.h"

#include "bcm2837.h"

// timer_freq = (2^31)/TIMER_PRESCALE * input_freq
// I believe the input frequency is the value in CNTFRQ_EL0
#define TIMER_PRESCALE ((volatile u32 *)(LOCAL_PBASE+0x08))

#define TIMER_LO       ((volatile u32 *)(LOCAL_PBASE+0x1C))
#define TIMER_HI       ((volatile u32 *)(LOCAL_PBASE+0x20))

#define TIMER_IRQ_CTL0 ((volatile u32 *)(LOCAL_PBASE+0x40)) // core 0
#define TIMER_IRQ_CTL1 ((volatile u32 *)(LOCAL_PBASE+0x44)) // core 1
#define TIMER_IRQ_CTL2 ((volatile u32 *)(LOCAL_PBASE+0x48)) // core 2
#define TIMER_IRQ_CTL3 ((volatile u32 *)(LOCAL_PBASE+0x4C)) // core 3

#define CNTP_CTL_EL0_ENABLE (1 << 0) // enable physical timer

static u64 interval;

void
handletimer(void)
{
    st_cntptval(interval);
}

void
timerinit(void)
{
    u64 freq = cntfrq(); // system counter frequency in Hz
    interval = freq / 100; // 100 times second;

    // scale the timer by 1: 2^31 = 0x80000000, therefore timer_freq = input_freq
    *TIMER_PRESCALE = 0x80000000;

    st_cntptval(interval);
    st_cntpctl(CNTP_CTL_EL0_ENABLE);
    *TIMER_IRQ_CTL0 = TIMER_IRQ_CTL_nCNTPNSIRQ;

    // TODO: configure CNTKCTL_EL1 to disable timer access in EL0

    //intenable(IRQ_TIMER);
}
